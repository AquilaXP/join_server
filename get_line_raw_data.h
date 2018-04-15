#pragma once

#include <cstdint>

#include <string>

/// Делит сырые данные на строки
class get_line_raw_data
{
public:
    void push_data( const char* data, size_t size )
    {
        m_buffer.append( data, size );
    }

    bool get_line( std::string& line )
    {
        auto b = m_buffer.find_first_of( '\n', m_procced_symbol );
        if( b == m_buffer.npos )
            return false;

        line.assign( &m_buffer[m_procced_symbol], b - m_procced_symbol );
        m_procced_symbol = b + 1;
        update();

        return true;
    }
private:
    void update()
    {
        std::move( m_buffer.begin() + m_procced_symbol, m_buffer.end(), m_buffer.begin() );
        m_buffer.erase( m_buffer.size() - m_procced_symbol );
        m_procced_symbol = 0;
    }

    uint32_t m_procced_symbol = 0;
    std::string m_buffer;
};