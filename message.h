#pragma once

#include <string>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/serialization.hpp>

constexpr int port = 42069;
constexpr int max_size = 1024;

struct message
{
  std::string type;
  std::string key;
  std::string value;

  template<typename Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & type;
    ar & key;
    ar & value;
  }
};
