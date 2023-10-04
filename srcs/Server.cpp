#include "Server.hpp"

Server::Server(int port) : _port(port), _socket(0), _kq(0) {}
Server::~Server() {} 

Server::Server(const Server& origin) { *this = origin; }
Server&	Server::operator=(const Server& origin) {
	if (this != &origin) {
		_port = origin.getPort();
	}
	return *this;
}

const int&	Server::getPort(void) const { return _port; }

void	Server::change_events(std::vector<struct kevent>& change_list, uintptr_t ident, \
						int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata) {
	struct kevent	tmp;

	EV_SET(&tmp, ident, filter, flags, fflags, data, udata);
	change_list.push_back(tmp);
}

void	Server::disconnect_client(int client_fd) {
	std::cout << "Client disconnected: " << client_fd << std::endl;
	close(client_fd);
	_clients.erase(client_fd);
}

void	Server::init(void) {
	_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (_socket == -1) throw std::runtime_error("Error: socket failed.");

	_server_addr.sin_family = AF_INET;
	_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_server_addr.sin_port = htons(_port);

	_kq = kqueue();
	if (_kq == -1) throw std::runtime_error("Error: kqueue fail.");
}

void	Server::run(void) {
	if (bind(_socket, (struct sockaddr*)&_server_addr, sizeof(_server_addr)) == -1) throw std::runtime_error("Error: bind failed.");
	if (listen(_socket, 5) == -1) throw std::runtime_error("Error: listen fail.");
	fcntl(_socket, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	
	struct kevent	event_list[8];

	change_events(_change_list, _socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	std::cout << "Server started." << std::endl;

	int	new_event;
	struct kevent*	curr_event;
	while (1) {
		new_event = kevent(_kq, &_change_list[0], _change_list.size(), event_list, 8, NULL);
		if (new_event == -1) throw std::runtime_error("Error: kevent fail.");

		_change_list.clear();

		for (int i = 0;i < new_event;++i) {
			curr_event = &event_list[i];

			if (curr_event->flags & EV_ERROR) {
				if (curr_event->ident == static_cast<uintptr_t>(_socket)) throw std::runtime_error("Error: server socket error.");
				else {
					std::cerr << "client socket error" << std::endl;
					disconnect_client(curr_event->ident);
				}
			} else if (curr_event->filter == EVFILT_READ) {
				if (curr_event->ident == static_cast<uintptr_t>(_socket)) {
					int	client_socket;

					if ((client_socket = accept(_socket, NULL, NULL)) == -1) throw std::runtime_error("Error: accept fail");
					std::cout << "accept new client: " << client_socket << std::endl;
					fcntl(client_socket, F_SETFL, O_NONBLOCK);

					change_events(_change_list, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					change_events(_change_list, client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					_clients[client_socket] = "";
				} else if (_clients.find(curr_event->ident) != _clients.end()) {
					char	buf[1024];
					int		n = read(curr_event->ident, buf, sizeof(buf));

					if (n <= 0) {
						if (n < 0) std::cerr << "Error: Client read error." << std::endl;
						disconnect_client(curr_event->ident);
					} else {
						buf[n] = '\0';
						_clients[curr_event->ident] += buf;
						std::cout << "received data from " << curr_event->ident << ": " << _clients[curr_event->ident] << std::endl;
					}
				}
			} else if (curr_event->filter == EVFILT_WRITE) {
				std::map<int, std::string>::iterator	it = _clients.find(curr_event->ident);
				if (it != _clients.end()) {
					if (_clients[curr_event->ident] != "") {
						int	n;

						if ((n = write(curr_event->ident, _clients[curr_event->ident].c_str(), _clients[curr_event->ident].size()) == -1)) {
							std::cerr << "Error: Client write error." << std::endl;
							disconnect_client(curr_event->ident);
						} else _clients[curr_event->ident].clear();
					}
				}
			}
		}
	}
}