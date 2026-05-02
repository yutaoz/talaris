# talaris
A simple, header only library for concurrent api polling

## How to
See main.cpp for example.

talaris does the polling on its own thread, so the main/consumer thread has to implement a function for the poller to sink the message body to.
Sink must take a std::string and return void.

Requires https://github.com/boostorg/beast

## Todo
- If blocking get proves to be a bottleneck, maybe an async implementation will be an improvement.
- POST request soon!
