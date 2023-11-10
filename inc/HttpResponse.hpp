#ifndef HTTPRESPONSE_HPP_
#define HTTPRESPONSE_HPP_

#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <iostream>

#include "HttpDecoder.hpp"
#include "HttpRequest.hpp"
#include "RouteRule.hpp"
#include "CgiHandler.hpp"
#include "HttpDecoderEnums.h"

#define RESPONSE_BUF_SIZE 134217728

class Client;

class HttpResponse {
  private:

    // start line
    unsigned short                            _http_major;
    unsigned short                            _http_minor;

    unsigned short                            _status;
    std::string                               _status_message;

    // headers
    std::map<std::string, std::string>        _headers;
    unsigned long long                        _content_length;
    bool                                      _is_chunked;

    // body
    std::vector<char>                         _body;

    bool                                      _is_ready;
    bool                                      _is_cgi;
    CgiHandler                                _cgi_handler;

    HPS::Method                               _method;
    int                                       _entity_idx;

    std::map<std::string, std::string>        _contentTypes;

    void                                      readDir(const std::string& path);

  public:
    HttpResponse(const HttpRequest& req, const RouteRule& route_rule);

    void                                      initContentTypes(void);
    const unsigned short&                     getHttpMajor(void) const;
    const unsigned short&                     getHttpMinor(void) const;

    const unsigned short&                     getStatus(void) const;
    const std::string&                        getStatusMessage(void) const;

    const std::map<std::string, std::string>& getHeader(void) const;
    const unsigned long long&                 getContentLength(void) const;
    const bool&                               getIsChunked(void) const;
    const std::vector<char>&                  getBody(void) const;
    const bool&                               getIsReady(void) const;
    const bool&                               getIsCgi(void) const;
    const int&                                getCgiPipeIn(void) const;
    CgiHandler&                               getCgiHandler(void);
    HPS::Method                               getMethod(void) const;
    const int&                                getEntityIdx(void) const;

    void                                      setHttpMajor(unsigned short http_major);
    void                                      setHttpMinor(unsigned short http_minor);
    void                                      setStatus(unsigned short status);
    void                                      setStatusMessage(const std::string& status_message);
    void                                      setHeaders(const std::map<std::string, std::string>& headers);
    void                                      setContentLength(unsigned long long content_length);
    void                                      setIsChunked(bool is_chunked);
    void                                      setBody(const std::vector<char>::const_iterator& it_begin, const std::vector<char>::const_iterator& it_end);

    void                                      setIsReady(bool is_ready);
    void                                      setIsCgi(bool is_cgi);
    void                                      setEntityIdx(int entity_idx);

    void                                      addContentLength(void);
    bool                                      isDir(const std::string& location);

    void                                      publish(const HttpRequest& req, const RouteRule*, const Client& client);
    void                                      publishError(int status, const RouteRule*, enum HPS::Method method);
    void                                      setHeader(const std::string& key, const std::string& value); 

    void                                      initializeCgiProcess(const HttpRequest& req, const RouteRule& rule, \
                                                  const std::string& server_name, const int& port, const int& client_fd) throw(std::runtime_error);
    int                                       cgiExecute(void) throw(std::runtime_error);
    
    void                                      readFile(const std::string& path);

    class FileNotFoundException : public std::exception {
      public: 
        const char* what() const throw() { return "File not found!"; }
    };
};



#endif
