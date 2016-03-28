#include "fixed_degree_task.h"
#include <system_error>
#include <boost/graph/copy.hpp>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

namespace gr
{

fixed_degree_task::fixed_degree_task(const undirected_graph& graph, double mu, int step_count, size_t graph_step)
  : base_task(graph, mu, step_count, graph_step)
{
  std::cout << "started: mu = " << mu << " step count = " << step_count << std::endl;
}

fixed_degree_task::~fixed_degree_task()
{
  std::cout << "finished: mu = " << mu_ << " step count = " << initial_step_count_ << std::endl;
}

void fixed_degree_task::make_randomization_step()
{
  edge e1 = boost::random_edge(graph_, rand_generator_);
  edge e2 = boost::random_edge(graph_, rand_generator_);
  vertex vs1 = boost::source(e1, graph_);
  vertex vt1 = boost::target(e1, graph_);
  vertex vs2 = boost::source(e2, graph_);
  vertex vt2 = boost::target(e2, graph_);
  while(e1 == e2 || vs1 == vs2 || vt1 == vt2 || vs1 == vt2 || vt1 == vs2 ||
    boost::edge(vs1, vs2, graph_).second ||
    boost::edge(vt1, vt2, graph_).second)
  {
    e1 = boost::random_edge(graph_, rand_generator_);
    vs1 = boost::source(e1, graph_);
    vt1 = boost::target(e1, graph_);
    e2 = boost::random_edge(graph_, rand_generator_);
    vs2 = boost::source(e2, graph_);
    vt2 = boost::target(e2, graph_);
  }
  assert(e1 != e2 && vs1 != vs2 && vt1 != vt2 &&
    !boost::edge(vs1, vs2, graph_).second &&
    !boost::edge(vt1, vt2, graph_).second);

  // removing edges
  boost::remove_edge(e1, graph_);
  boost::remove_edge(e2, graph_);
  assert(!boost::edge(vs1, vt1, graph_).second &&
    !boost::edge(vs2, vt2, graph_).second);
  
  int removed = 0;
  int added = 0;

  int removed1 = 0;
  int added1 = 0;

  std::future<void> result = pool_.enqueue([&]()
  {
    adjacency_iterator v, v_end;
    for(boost::tie(v, v_end) = boost::adjacent_vertices(vs1, graph_); v != v_end; ++v)
    {
      if(*v != vt1 && boost::edge(*v, vt1, graph_).second)
      {
        ++removed1;
      }
      if(*v != vs2 && boost::edge(*v, vs2, graph_).second)
      {
        ++added1;
      }
    }
  });

  adjacency_iterator v, v_end;
  for(boost::tie(v, v_end) = boost::adjacent_vertices(vt2, graph_); v != v_end; ++v)
  {
    if(*v != vs2 && boost::edge(*v, vs2, graph_).second)
    {
      ++removed;
    }
    if(*v != vt1 && boost::edge(*v, vt1, graph_).second)
    {
      ++added;
    }
  }

  result.get();

  added += added1;
  removed += removed1;
  boost::add_edge(vs1, vs2, graph_);
  boost::add_edge(vt1, vt2, graph_);
  int delta = added - removed;
  if(check_step(delta))
  {
    num_triangles_ += delta;
    return; 
  }
  boost::add_edge(vs1, vt1, graph_);
  boost::add_edge(vs2, vt2, graph_);
  boost::remove_edge(vs1, vs2, graph_);
  boost::remove_edge(vt1, vt2, graph_);
}

}
