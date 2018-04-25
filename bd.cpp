#include "bd.h"

#include <cstring>

void table::insert( uint32_t id, const std::string& name )
{
    auto find_id = m_index.find( id );
    if( find_id != m_index.end() )
        throw std::runtime_error( "duplicate " + std::to_string( id ) );

    m_table_data.emplace_back( std::make_pair( id, name ) );
}

void table::truncate()
{
    m_index.clear();
    m_table_data.clear();
}

uint32_t table::size() const
{
    return m_table_data.size();
}

std::vector<std::tuple<uint32_t, std::string, std::string >> table::intersection( const table& t1, uint32_t size1, const table& t2, uint32_t size2 )
{
    auto index1 = t1.generate_index( size1 );
    auto index2 = t2.generate_index( size2 );

    std::vector<std::tuple<uint32_t, std::string, std::string >> res;
    auto first1 = index1.begin();
    auto last1 = index1.end();

    auto first2 = index2.begin();
    auto last2 = index2.end();

    while( first1 != last1 && first2 != last2 )
    {
        if( first1->first < first2->first )
            ++first1;
        else
        {
            if( !( first2->first < first1->first ) )
            {
                res.emplace_back( std::make_tuple( first1->first, first1->second->second, first2->second->second ) );
                ++first1;
            }
            ++first2;
        }
    }

    return res;
}

std::vector<std::tuple<uint32_t, std::string, std::string>> table::symmetric_difference( const table& t1, uint32_t size1, const table& t2, uint32_t size2 )
{
    std::vector<std::tuple<uint32_t, std::string, std::string>> result;
    auto index1 = t1.generate_index( size1 );
    auto index2 = t2.generate_index( size2 );

    auto first1 = index1.begin();
    auto last1 = index1.end();

    auto first2 = index2.begin();
    auto last2 = index2.end();
    while( first1 != last1 )
    {
        if( first2 == last2 )
        {
            for( ; first1 != last1; ++first1 )
            {
                result.push_back( std::make_tuple( first1->first, first1->second->second, std::string() ) );

            }
            break;
        }

        if( first1->first < first2->first ){
            result.push_back( std::make_tuple( first1->first, first1->second->second, std::string() ) );
            ++first1;
        }
        else
        {
            if( first2->first < first1->first )
            {
                result.push_back( std::make_tuple( first2->first, std::string(), first2->second->second ) );
            }
            else{
                ++first1;
            }
            ++first2;
        }
    }

    for( ; first2 != last2; ++first2 )
    {
        result.push_back( std::make_tuple( first2->first, std::string(), first2->second->second ) );
    }

    return result;
}

std::vector < std::pair<uint32_t, std::string> > table::get_table_data() const
{
    std::vector < std::pair<uint32_t, std::string> > res;
    res.reserve( m_table_data.size() );
    std::copy( m_table_data.begin(), m_table_data.end(), std::back_inserter( res ) );
    return res;
}

std::map<uint32_t, const std::pair<uint32_t, std::string>*> table::generate_index( uint32_t count ) const
{
    std::map<uint32_t, const std::pair<uint32_t, std::string>*> index;
    uint32_t i = 0;
    for( auto iter = m_table_data.begin(); i < count; ++i, ++iter )
    {
        index.insert( { iter->first, &( *iter ) } );
    }
    return index;
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
        m_A->insert( id, name );
    else if( table_name == "B" )
        m_B->insert( id, name );
}

void bd::truncate( const std::string& table_name )
{
    lock_t lk( m_mutex );
    if( not ( table_name == "A" || table_name == "B" ) )
        throw std::runtime_error( "incorect name table: only A or B" );

    if( table_name == "A" )
        m_A = std::make_shared<table>();
    else if( table_name == "B" )
        m_B = std::make_shared<table>();
}

std::vector<std::tuple<uint32_t, std::string, std::string>> bd::intersection()
{
    uint32_t sizeA = 0;
    uint32_t sizeB = 0;
    std::shared_ptr<table> save_tableA;
    std::shared_ptr<table> save_tableB;
    {
        lock_t lk( m_mutex );
        save_tableA = m_A;
        save_tableB = m_B;
        sizeA = m_A->size();
        sizeB = m_B->size();
    }
    return table::intersection( *save_tableA, sizeA, *save_tableB, sizeB );
}

std::vector<std::tuple<uint32_t, std::string, std::string>> bd::symmetric_difference()
{
    uint32_t sizeA = 0;
    uint32_t sizeB = 0;
    std::shared_ptr<table> save_tableA;
    std::shared_ptr<table> save_tableB;
    {
        lock_t lk( m_mutex );
        save_tableA = m_A;
        save_tableB = m_B;
        sizeA = m_A->size();
        sizeB = m_B->size();
    }
    return table::symmetric_difference( *save_tableA, sizeA, *save_tableB, sizeB );
}

std::vector<std::pair<uint32_t, std::string>> bd::select( const std::string& table_name )
{
    lock_t lk( m_mutex );
    if( not ( table_name == "A" || table_name == "B" ) )
        throw std::runtime_error( "incorect name table: only A or B" );

    if( table_name == "A" )
        return m_A->get_table_data();
    else if( table_name == "B" )
        return m_B->get_table_data();
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
