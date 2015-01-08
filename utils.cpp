#include "utils.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/text_iarchive.hpp> 
#include <boost/archive/text_oarchive.hpp> 
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

#include <sstream>

namespace gr
{

namespace utils
{

void copy_graph(const undirected_graph& input, undirected_graph& output)
{
  std::stringstream ss;
  archive::text_oarchive oa(ss);
  serialization::save(oa, input, 0);
  archive::text_iarchive ia(ss);
  serialization::load(ia, output, 0);
}

void serialize_graph(const undirected_graph& graph, std::string& str)
{
  std::stringstream ss;
  archive::text_oarchive oa(ss);
  serialization::save(oa, graph, 0);
  str = ss.str();
}

void deserialize_graph(const std::string& str, undirected_graph& graph)
{
  std::stringstream ss(str);
  archive::text_iarchive ia(ss);
  serialization::load(ia, graph, 0);
}

void compress_string(const std::string& input, std::string& output)
{
  namespace io = boost::iostreams;
  std::stringstream ss(input);
  io::filtering_streambuf<io::input> fs;
  fs.push(io::bzip2_compressor());
  fs.push(ss);
  std::stringstream zss;
  io::copy(fs, zss);
  output = zss.str();
}

void decompress_string(const std::string& input, std::string& output)
{
  namespace io = boost::iostreams;
  std::stringstream zss(input);
  io::filtering_streambuf<io::input> fs;
  fs.push(io::bzip2_decompressor());
  fs.push(zss);
  std::stringstream ss;
  io::copy(fs, ss);
  output = ss.str();
}

}

}
