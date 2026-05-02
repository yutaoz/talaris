#include <iostream>
#include <chrono>
#include "talaris/tala.hpp"

void handle_response(std::string body) {
  std::cout << "Received " << body.size() << " bytes\n";
}

int main() {

  tala::Poller poller{"api.hypixel.net", 
                      "443",
                      std::chrono::seconds(1),
                      handle_response,
                      };

  poller.start("/v2/skyblock/bazaar");
  std::cout << "Press Enter to stop polling...\n";
  std::string line;
  std::getline(std::cin, line);

  poller.stop();

  std::cout << "Stopped. Press Enter to exit...\n";
  std::getline(std::cin, line);

  
}
