#include <iostream>
#include <optional>
#include <boost/asio.hpp>
#include "message.h"

using boost::asio::ip::tcp;

class Client
{
public:
  explicit Client(const std::string& hostname)
    : hostname(hostname) {}

  void connect()
  {
    boost::asio::connect(socket, resolver.resolve(hostname, std::to_string(port)));
  }

  message generate_msg(int n, char* argv[])
  {
    if (n == 5)
    {
      // { <set>, <key>, <value> }
      return { argv[2], argv[3], argv[4] };
    }

    // { <get>, <key> }
    return { argv[2], argv[3] };
  }

  std::optional<std::string> send_cmd(const message& msg)
  {
    boost::asio::streambuf buf;
    std::ostream os(&buf);
    boost::archive::binary_oarchive out_archive(os);
    out_archive << msg;
    boost::asio::write(socket, buf);
    
    boost::asio::streambuf recv_buf;
    boost::system::error_code error;
    char reply[1024];
    std::size_t transferred = socket.read_some(boost::asio::buffer(reply), error);

    if (transferred)
    {
      return reply;
    }

    return std::nullopt;
  }

private:
  boost::asio::io_context context;
  tcp::socket socket{context};
  tcp::resolver resolver{context};
  std::string hostname;
};

int main(int argc, char* argv[])
{
  if (argc < 4)
    {
      std::cerr << "usage: " << argv[0] << " <host> <set/get> <key> (<value>)"
                << std::endl;
      return 1;
    }

  try
    {
      Client client(argv[1]);
      client.connect();

      const auto msg = client.generate_msg(argc, argv);
      const auto reply = client.send_cmd(msg);

      if (reply)
      {
        std::cout << *reply << std::endl;
      }
    }
  catch (std::exception& e)
    {
      std::cerr << "error: " << e.what() << std::endl;
    }
}
