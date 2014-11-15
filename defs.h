#ifndef _DEFS_H_
#define _DEFS_H_

#include <random>
#include <boost/mpi.hpp>
#include <boost/program_options.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/filesystem.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/accumulators.hpp>

namespace archive = boost::archive;
namespace serialization = boost::serialization;
namespace mpi = boost::mpi;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace ac = boost::accumulators;

namespace graph_randomization
{

// undirected graph
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS> undirected_graph;
// random generator
typedef std::mt19937 random_generator;
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

}

#endif // _DEFS_H_
