#include "bd.h"

#include <cstring>

void table::insert( uint32_t id, const std::string& name )
{
    auto res = m_data.insert( { id,name } );
    if( res.second == false )
        throw std::runtime_error( "duplicate " + std::to_string( id ) );
}

void table::truncate()
{
    m_data.clear();
}

std::vector<std::tuple<uint32_t, std::string, std::string >> table::intersection( const table& t1, const table& t2 )
{
    std::vector<std::pair<uint32_t, std::string >> result;
    std::set_intersection( t1.m_data.begin(), t1.m_data.end(), t2.m_data.begin(), t2.m_data.end(), std::back_inserter( result ),
        []( const std::pair<uint32_t, std::string >& p1, const std::pair<uint32_t, std::string >& p2 )
    {
        return p1.first < p2.first;
    } );

    std::vector<std::tuple<uint32_t, std::string, std::string >> res;
    for( auto& r : result )
    {
        res.push_back( std::make_tuple( r.first, t1.m_data.find( r.first )->second, t2.m_data.find( r.first )->second ) );
    }
    return res;
}

std::vector<std::tuple<uint32_t, std::string, std::string>> table::symmetric_difference( const table& t1, const table& t2 )
{
    std::vector<std::tuple<uint32_t, std::string, std::string>> result;
    auto first1 = t1.m_data.begin();
    auto last1 = t1.m_data.end();

    auto first2 = t2.m_data.begin();
    auto last2 = t2.m_data.end();
    while( first1 != last1 )
    {
        if( first2 == last2 )
        {
            for( ; first1 != last1; ++first1 )
            {
                result.push_back( std::make_tuple( first1->first, first1->second, std::string() ) );

            }
            break;
        }

        if( first1->first < first2->first ){
            result.push_back( std::make_tuple( first1->first, first1->second, std::string() ) );
            ++first1;
        }
        else
        {
            if( first2->first < first1->first )
            {
                result.push_back( std::make_tuple( first2->first, std::string(), first2->second ) );
            }
            else{
                ++first1;
            }
            ++first2;
        }
    }

    for( ; first2 != last2; ++first2 )
    {
        result.push_back( std::make_tuple( first2->first, std::string(), first2->second ) );
    }

    return result;
}

std::vector < std::pair<uint32_t, std::string> > table::get_table_data() const
{
    std::vector < std::pair<uint32_t, std::string> > res;
    res.reserve( m_data.size() );
    std::copy( m_data.begin(), m_data.end(), std::back_inserter( res ) );
    return res;
}

bd& bd::instance()
{
    static bd single;
    return single;
}

void bd::insert( const std::string& table_name, uint32_t id, const std::string& name )
{
    lock_t lk( m_mutex );
    if( not ( table_name == "A" || table_name == "B" ) )
        throw std::runtime_error( "incorect name table: only A or B" );

    if( table_name == "A" )
        m_A.insert( id, name );
    else if( table_name == "B" )
        m_B.insert( id, name );
}

void bd::truncate( const std::string& table_name )
{
    lock_t lk( m_mutex );
    if( not ( table_name == "A" || table_name == "B" ) )
        throw std::runtime_error( "incorect name table: only A or B" );

    if( table_name == "A" )
        m_A.truncate();
    else if( table_name == "B" )
        m_B.truncate();
}

std::vector<std::tuple<uint32_t, std::string, std::string>> bd::intersection()
{
    lock_t lk( m_mutex );
    return table::intersection( m_A, m_B );
}

std::vector<std::tuple<uint32_t, std::string, std::string>> bd::symmetric_difference()
{
    lock_t lk( m_mutex );
    return table::symmetric_difference( m_A, m_B );
}

std::vector<std::pair<uint32_t, std::string>> bd::select( const std::string& table_name )
{
    lock_t lk( m_mutex );
    if( not ( table_name == "A" || table_name == "B" ) )
        throw std::runtime_error( "incorect name table: only A or B" );

    if( table_name == "A" )
        return m_A.get_table_data();
    else if( table_name == "B" )
        return m_B.get_table_data();
}

parser_commands::parser_commands()
{

}

void parser_commands::send_data( std::function<void( const char*, std::size_t )> delegate )
{
    m_delegate = delegate;
}

void parser_commands::push_command( const std::string& cmd )
{
    try
    {
        std::istringstream ss( cmd );
        std::string key_word;
        std::string end;
        ss >> key_word;
        if( key_word == "INSERT" )
        {
            std::string table_name;
            std::string id_str;
            std::string name;
            ss >> table_name >> id_str >> name;
            uint32_t id;
            try
            {
                if( id_str.find_first_not_of( "-+0123456789" ) != std::string::npos )
                    throw std::runtime_error( "incorect command INSERT" );
                id = std::stol( id_str );
            }
            catch( const std::exception& )
            {
                throw std::runtime_error( "incorect command INSERT" );
            }

            ss >> end;
            if( !end.empty() )
                throw std::runtime_error( "incorect command INSERT" );

            bd::instance().insert( table_name, id, name );
            send_ok();
        }
        else if( key_word == "TRUNCATE" )
        {
            std::string table_name;
            ss >> table_name;
            ss >> end;
            if( !end.empty() )
                throw std::runtime_error( "incorect command TRUNCATE" );

            bd::instance().truncate( table_name );
            send_ok();
        }
        else if( key_word == "INTERSECTION" )
        {
            ss >> end;
            if( !end.empty() )
                throw std::runtime_error( "incorect command INTERSECTION" );

            auto result = bd::instance().intersection();
            if( m_delegate )
            {
                std::string line;
                std::ostringstream ss;

                for( auto& r : result )
                {
                    ss.str( std::string() );
                    print( ss, r );
                    auto line = ss.str();
                    m_delegate( line.c_str(), line.size() );
                }
                send_ok();
            }
        }
        else if( key_word == "SYMMETRIC_DIFFERENCE" )
        {
            ss >> end;
            if( !end.empty() )
                throw std::runtime_error( "incorect command SYMMETRIC_DIFFERENCE" );

            auto result = bd::instance().symmetric_difference();
            if( m_delegate )
            {
                std::string line;
                std::ostringstream ss;

                for( auto& r : result )
                {
                    ss.str( std::string() );
                    print( ss, r );
                    auto line = ss.str();
                    m_delegate( line.c_str(), line.size() );
                }
                send_ok();
            }
        }
        else if( key_word == "SELECT" )
        {
            std::string table_name;
            ss >> table_name;

            ss >> end;
            if( !end.empty() )
                throw std::runtime_error( "incorect command TRUNCATE" );

            auto r = bd::instance().select( table_name );
            if( m_delegate )
            {
                std::ostringstream ss;
                for( auto& l : r )
                {
                    ss.str( std::string() );
                    ss << l.first << ' ' << l.second << '\n';
                    auto line = ss.str();
                    m_delegate( line.c_str(), line.size() );
                }
            }
            send_ok();
        }
        else
        {
            throw std::runtime_error( "incorect command" );
        }
    }
    catch( const std::exception& err )
    {
        send_err( err.what() );
    }
}

void parser_commands::send_ok()
{
    m_delegate( "OK\n", 3 );
}

void parser_commands::send_err( const char* err_what )
{
    m_delegate( "ERR ", 4 );
    m_delegate( err_what, strlen( err_what ) );
    m_delegate( "\n", 1 );
}

void parser_commands::send( const std::string& value )
{
    m_delegate( value.c_str(), value.size() );
}
