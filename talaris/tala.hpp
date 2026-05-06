#ifndef TALA_H
#define TALA_H

#include <chrono>
#include <thread>

#include <boost/asio/steady_timer.hpp>

#include "http/http_req.hpp"


namespace tala {

template <typename Sink>
class Poller {

public:
  Poller( std::string host, 
          std::string port, 
          std::chrono::milliseconds delay, 
          Sink sink
        ) : 
          endpoint(std::move(host), std::move(port)), 
          client(endpoint, ioc),
          timer(ioc),
          delay(delay),
          sink(std::move(sink)) {}

  void start(const std::string& target) {
    worker = std::jthread(
        [this, target](std::stop_token st) {
          run(st, target);
        });
  }

  void stop() {
    if (worker.joinable()) {
      worker.request_stop();

      timer.cancel();
    }
  }
  
  void run(std::stop_token st, const std::string& target) {

    try {
      client.connect();

      auto next_poll = std::chrono::steady_clock::now();

      while (!st.stop_requested()) {
        next_poll += delay;

        try {
          auto res = client.get(target);

          if (res.result() != http::status::ok) {
            std::cerr << "HTTP error: " << res.result_int() << "\n";
          } else {
            sink(std::move(res.body()));
          }
        } catch (const std::exception& e) {
          std::cerr << "poll failed: " << e.what() << "\n";

          try {
            client.close();
          } catch (...) {}

          if (st.stop_requested()) {
            break;
          }

          try {
            client.connect();
          } catch (const std::exception& reconnect_error) {
            std::cerr << "reconnect failed: " << reconnect_error.what() << "\n";
          }
        }

        timer.expires_at(next_poll);

        boost::system::error_code ec;
        timer.wait(ec);

        if (ec) {
          std::cerr << "timer error: " << ec.message() << "\n";
          break;
        }
      }

      client.close();
    } catch (const std::exception& e) {
      std::cerr << "poller stopped: " << e.what() << "\n";
    }

  }

private:
  HttpEndpoint endpoint;
  net::io_context ioc;
  HttpClient client;
  std::jthread worker;
  std::chrono::milliseconds delay;
  boost::asio::steady_timer timer;
  Sink sink;
};

}

#endif
