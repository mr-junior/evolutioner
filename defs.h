#pragma once

#include <boost/mpi.hpp>
#include <boost/program_options.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/filesystem.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/accumulators.hpp>

#include <boost/archive/text_oarchive.hpp> 
#include <boost/archive/text_iarchive.hpp> 
#include <boost/graph/adj_list_serialize.hpp>

#include <random>

namespace archive = boost::archive;
namespace serialization = boost::serialization;
namespace mpi = boost::mpi;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace ac = boost::accumulators;

namespace gr
{

// random generator
typedef std::mt19937 random_generator;

// undirected graph
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS> undirected_graph;
// Erdos-Renyi generator
typedef boost::sorted_erdos_renyi_iterator<random_generator, undirected_graph> erdos_renyi_iterator;
// graph vertex
typedef boost::graph_traits<undirected_graph>::vertex_descriptor vertex;
// graph edge
typedef boost::graph_traits<undirected_graph>::edge_descriptor edge;
// graph out edge
typedef boost::graph_traits<undirected_graph>::out_edge_iterator out_edge_iterator;
// graph edge
typedef boost::graph_traits<undirected_graph>::edge_iterator edge_iterator;
// vertex iterator
typedef boost::graph_traits<undirected_graph>::vertex_iterator vertex_iterator;
// adjacency list iterator
typedef boost::graph_traits<undirected_graph>::adjacency_iterator adjacency_iterator;

struct graph_data
{
  graph_data(const undirected_graph& graph) :
    graph_(graph)
  {
  }

  undirected_graph graph_;

  virtual std::string get_prop_value(const std::string& prop_name) const
  {
    return std::string();
  }
  virtual std::string get_description() const
  {
    return std::string();
  }
  virtual void serialize(const std::string& file_name)
  {
    std::ofstream file;
    file.open(file_name.c_str());
    if(!file.is_open())
    {
      std::cerr << "Failed to open/create output file." << std::endl;
      return;
    }
    archive::text_oarchive oa(file);
    oa << boost::num_vertices(graph_);
    const std::string& header = get_header();
    if(!header.empty())
    {
      oa << header;
    }
    serialization::save(oa, graph_, 0);
    const std::string& footer = get_footer();
    if(!footer.empty())
    {
      oa << footer;
    }
    file.close();
  }

private:
  virtual std::string get_header() const
  {
    return std::string();
  }
  virtual std::string get_footer() const
  {
    return std::string();
  }
};

struct er_graph_data : graph_data
{
  er_graph_data(const undirected_graph& graph, std::size_t N, double p) :
    graph_data(graph),
    probability_(p)
  {
  }

  double probability_;
  virtual std::string get_prop_value(const std::string& prop_name) const
  {
    if(prop_name == "p")
    {
      //return std::to_string(probability_);
      std::stringstream ss;
      ss << probability_;
      return ss.str();
    }
    return std::string();
  }
  virtual std::string get_description() const
  {
    return std::string("p") + get_prop_value("p");
  }
  virtual std::string get_header() const
  {
    return get_prop_value("p");
  }
};

}
