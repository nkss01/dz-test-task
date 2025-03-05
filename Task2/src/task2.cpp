#include <boost/asio.hpp>
#include "server.hpp"
#include "config.h"

void start()
{
    const std::string cnStr = DATABASE_CONNECTION;

    net::io_context ioc{ 1 };
    tcp::endpoint endpoint(tcp::v4(), SERVER_PORT);
    HttpServer server(ioc, endpoint, cnStr, MAX_CONNECTIONS);
    std::cout << "Server listening on port " << SERVER_PORT << "...\n";

    ioc.run();
}

int main()
{
    try
    {
        start();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}