#pragma once

#include <deque>

#include <boost/asio.hpp>

#include "get_line_raw_data.h"
#include "bd.h"

const uint32_t size_block_read = 104;

class server;
/// Обработчик одного соединения
class handler_connect
{
public:
    handler_connect( server* server, boost::asio::io_service& io_service,  boost::asio::ip::tcp::socket&& socket );
    ~handler_connect();
    void do_write( const char* data, size_t size );
    void do_write();
private:
    void do_read();

    void disconnect();

    bool is_del = false;
    std::deque<std::string> m_out;
    char m_buffer[size_block_read];
    std::string m_line;
    server* m_server = nullptr;
    boost::asio::io_service& m_io_service;
    get_line_raw_data m_gltd;
    boost::asio::ip::tcp::socket m_socket;
    parser_commands m_pc;
};