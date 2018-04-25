#include "handler_connect.h"

#include "server.h"
#include "bd.h"

handler_connect::handler_connect( server* server, boost::asio::io_service& io_service, boost::asio::ip::tcp::socket&& socket )
    : m_server( server ), m_io_service( io_service ), m_socket( std::move( socket ) )
{
    m_pc.send_data( [this]( const char* data, size_t size ) {
        do_write( data, size );
    } );

    do_read();
}

handler_connect::~handler_connect()
{
    disconnect();
}

void handler_connect::do_write()
{
    boost::asio::async_write(
        m_socket,
        boost::asio::buffer( m_out.back().data(), m_out.back().length() ),
        [this]( boost::system::error_code ec, std::size_t )
    {
        if( !ec )
        {
            m_out.pop_front();
        }
        else
        {
            disconnect();
        }
    } );
}

void handler_connect::do_write( const char* data, size_t size )
{
    std::string msg( data, size );
    m_io_service.post( [this, msg]
    {
        m_out.push_back( std::move(msg) );
        do_write();
    } );
}

void handler_connect::do_read()
{
    boost::asio::async_read(
        m_socket, boost::asio::buffer( m_buffer, size_block_read ),
        boost::asio::transfer_at_least( 1 ),
        [this]( boost::system::error_code ec, std::size_t length )
    {
        /// Дочитываем в любой случае
        if( length != 0 )
            m_gltd.push_data( m_buffer, length );

        while( m_gltd.get_line( m_line ) )
        {
            m_pc.push_command( m_line );
        }
        if( ec || length == 0 )
        {
            disconnect();
        }
        else
        {
            do_read();
        }
    } );
}

void handler_connect::disconnect()
{
    if( !is_del )
    {
        is_del = true;
        m_socket.close();
        m_io_service.post( [this](){m_pc.send_data( nullptr ); } );
        m_server->remove_handler( this );
    }
}
