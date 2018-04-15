#include <iostream>

#include "server.h"

int main( int ac, char* av[] )
{
    int ret = 0;
    try
    {
        if( ac != 2 )
            throw std::runtime_error( "incorect command argumetns: 1 - port" );

        uint16_t port = std::atoi( av[1] );

        boost::asio::io_service io_service;

        server s( io_service,
            boost::asio::ip::tcp::endpoint( boost::asio::ip::address(), port ) );
        io_service.run();
    }
    catch( const std::exception& err )
    {
        std::cerr << err.what() << '\n';
        ret = 1;
    }
    return ret;
}