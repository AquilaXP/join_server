#pragma once

#include <cstdint>
#include <ciso646>

#include <string>
#include <iostream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <map>
#include <mutex>
#include <list>
#include <vector>
#include <memory>

/*
INSERT table id name
TRUNCATE table
INTERSECTION
SYMMETRIC_DIFFERENCE
*/
class table
{
public:
    void insert( uint32_t id, const std::string& name );
    void truncate();
    uint32_t size() const;
    static std::vector<std::tuple<uint32_t, std::string, std::string >> intersection( const table& t1, uint32_t size1, const table& t2, uint32_t size2 );
    static std::vector<std::tuple<uint32_t, std::string, std::string>> symmetric_difference( const table& t1, uint32_t size1, const table& t2, uint32_t size2 );
    std::vector < std::pair<uint32_t, std::string> > get_table_data() const;
private:
    std::map<uint32_t, const std::pair<uint32_t, std::string>*> generate_index( uint32_t count ) const;

    std::list<std::pair<uint32_t, std::string>> m_table_data;
    std::map<uint32_t, std::pair<uint32_t, std::string>* > m_index;
};

class bd
{
    typedef std::lock_guard<std::mutex> lock_t;
public:
    static bd& instance();
    void insert( const std::string& table_name, uint32_t id, const std::string& name );
    void truncate( const std::string& table_name );
    std::vector<std::tuple<uint32_t, std::string, std::string>> intersection();
    std::vector<std::tuple<uint32_t, std::string, std::string>> symmetric_difference();
    std::vector<std::pair<uint32_t, std::string>> select( const std::string& table_name );
private:
    bd() = default;

    std::mutex m_mutex;
    std::shared_ptr<table> m_A = std::make_shared<table>();
    std::shared_ptr<table> m_B = std::make_shared<table>();
};

template<std::size_t...> struct seq{};

template<std::size_t N, std::size_t... Is>
struct gen_seq : gen_seq<N - 1, N - 1, Is...>{};

template<std::size_t... Is>
struct gen_seq<0, Is...> : seq<Is...>{};

class parser_commands
{
public:
    parser_commands();
    void send_data( std::function<void(const char*,std::size_t)> delegate );
    void push_command( const std::string& cmd );
private:
    template< class ChStream, class TrStream, class Tuple, std::size_t... Is >
    void print_tuple_impl( std::basic_ostream<ChStream, TrStream>& os, const Tuple& t, seq<Is...> );

    template< class ChStream, class TrStream, class... Args >
    void print( std::basic_ostream<ChStream, TrStream>& os, const std::tuple<Args...>& t );
    void send_ok();
    void send_err( const char* err_what );
    void send( const std::string& value );

    std::function<void( const char*, std::size_t )> m_delegate;
};

template< class ChStream, class TrStream, class Tuple, std::size_t... Is >
void parser_commands::print_tuple_impl( std::basic_ostream<ChStream, TrStream>& os, const Tuple& t, seq<Is...> )
{
    auto b = { ( ( os << ( Is == 0 ? "" : "," ) , os << std::get<Is>( t ) ), 0 )... };
}

template< class ChStream, class TrStream, class... Args >
void parser_commands::print( std::basic_ostream<ChStream, TrStream>& os, const std::tuple<Args...>& t )
{
    print_tuple_impl( os, t, gen_seq<sizeof...( Args )>{} );
    os << '\n';
}
