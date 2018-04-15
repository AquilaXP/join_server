#include "server.h"

#include "bd.h"
#include "handler_connect.h"

server::server( boost::asio::io_service& io_service, const boost::asio::ip::tcp::endpoint& end_point ) : m_service( io_service ),
    m_acceptor( io_service, end_point ),
    m_socket( io_service )
{
    // иницируем создание БД
    bd::instance();
    do_accept();
}

void server::do_accept()
{
    m_acceptor.async_accept( m_socket,
        [this]( boost::system::error_code ec )
    {
        if( !ec )
        {
            m_handlers.insert( std::make_shared<handler_connect>
                ( this, m_service, std::move( m_socket ) ) );
        }
        do_accept();
    } );
}

void server::remove_handler( handler_connect* hc )
{
    m_service.post( [hc, this]{
        std::shared_ptr<handler_connect> s( hc, []( const handler_connect* ){} );
        m_handlers.erase( s );
    } );
}