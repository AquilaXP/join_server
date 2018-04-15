#pragma once

#include <boost/asio.hpp>

#include <set>
#include <memory>

/// Сервер
///
/// Ожидает соединения, создается обработчик на каждое соединение,
/// создается отдельный обработчик для обычных комманд
class handler_connect;

class server
{
    friend class handler_connect;
public:
    server( boost::asio::io_service& io_service,
        const boost::asio::ip::tcp::endpoint& end_point );

private:
    void do_accept();
    void remove_handler( handler_connect* hc );

    std::set<std::shared_ptr<handler_connect>> m_handlers;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::io_service& m_service;
};
