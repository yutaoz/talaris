#ifndef HTTP_REQ_H
#define HTTP_REQ_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <iostream>
#include <chrono>
#include <memory>
#include <string>
#include <utility>

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp; 

struct HttpEndpoint {
  std::string host;
  std::string port;

};

class HttpClient {

  HttpEndpoint endpoint;
  net::io_context& ioc;

  net::ssl::context ssl_ctx;
  tcp::resolver     resolver;

  std::unique_ptr<beast::ssl_stream<beast::tcp_stream>> stream;

public:

  HttpClient(HttpEndpoint endpoint, net::io_context& ioc)
  : endpoint(std::move(endpoint)),
    ioc(ioc),
    ssl_ctx(net::ssl::context::tls_client),
    resolver(ioc)
  {
    ssl_ctx.set_default_verify_paths();
    ssl_ctx.set_verify_mode(net::ssl::verify_peer);
  }

  void connect() {
    stream = std::make_unique<beast::ssl_stream<beast::tcp_stream>>(ioc, ssl_ctx);

    beast::get_lowest_layer(*stream).expires_after(std::chrono::seconds(5));
    auto const results = resolver.resolve(endpoint.host, endpoint.port);
    beast::get_lowest_layer(*stream).connect(results);

    if (!SSL_set_tlsext_host_name(stream->native_handle(), endpoint.host.c_str())) {
      beast::error_code ec(static_cast<int>(::ERR_get_error()),
                           net::error::get_ssl_category());
      throw beast::system_error(ec, "Failed to set SNI Hostname");
    }

    stream->handshake(net::ssl::stream_base::client);
  }

  http::response<http::string_body> get(const std::string& target) {
    beast::get_lowest_layer(*stream).expires_after(std::chrono::seconds(5));

    http::request<http::empty_body> req{http::verb::get, target, 11};

    req.set(http::field::host, endpoint.host);
    req.keep_alive(true);

    http::write(*stream, req);

    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    
    http::read(*stream, buffer, res);

    return res;
  }

  void close() {
    beast::error_code ec;
    stream->shutdown(ec);
    if (ec == net::error::eof) {
      ec = {};
    }
  }
};

#endif
