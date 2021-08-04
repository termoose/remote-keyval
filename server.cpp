#include <iostream>
#include <unordered_map>
#include <optional>
#include <boost/iostreams/stream.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/asio.hpp>
#include "message.h"

using boost::asio::ip::tcp;

class Cache
{
public:
  void put(const std::string& key, const std::string& value)
  {
    std::lock_guard<std::mutex> lock(data_mutex);
    data[key] = value;
  }

  std::optional<std::string> get(const std::string& key)
  {
    std::lock_guard<std::mutex> lock(data_mutex);

    auto found = data.find(key);
    if (found == data.cend())
    {
      return std::nullopt;
    }

    return found->second;
  }
  
private:
  std::mutex data_mutex;
  std::unordered_map<std::string, std::string> data;
};

class Server
{
public:
  explicit Server(Cache* cache_ptr)
    : cache_ptr(cache_ptr), acceptor(context, tcp::endpoint(tcp::v4(), port)) {}

  void work()
  {
    while (true)
    {
      tcp::socket socket = acceptor.accept();
      auto msg = get_message(socket);

      if ("put" == msg.type)
      {
        cache_ptr->put(msg.key, msg.value);
      }
      else if ("get" == msg.type)
      {
        const std::string reply = handle_get(msg);
        boost::asio::write(socket, boost::asio::buffer(reply, reply.size()));
      }
    }
  }

private:
  std::string handle_get(const message& msg)
  {
    const auto reply = cache_ptr->get(msg.key);
    
    if (reply)
    {
      return *reply;
    }

    return "error: not found";
  }

  message get_message(tcp::socket& socket)
  {
    boost::asio::streambuf buf;
    boost::system::error_code error;
    char data[1024];
    std::size_t length = socket.read_some(boost::asio::buffer(data), error);

    message msg;
    {
      boost::iostreams::basic_array_source<char> source(data, sizeof(data));
      boost::iostreams::stream<boost::iostreams::basic_array_source<char>> s(source);
      boost::archive::binary_iarchive in_archive(s);
      in_archive >> msg;
    }

    return msg;
  }

  Cache* cache_ptr;
  boost::asio::io_context context;
  tcp::acceptor acceptor;
};

int main()
{
  Cache cache;
  Server server(&cache);

  server.work();
}
