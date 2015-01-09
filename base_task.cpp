#include "base_task.h"
#include "utils.h"

#include <boost/archive/text_oarchive.hpp> 
#include <boost/archive/text_iarchive.hpp> 

#include <future>
#include <queue>

namespace gr
{

base_task::base_task(const undirected_graph& graph, double mu, size_t step_count, size_t graph_step) :
  graph_(graph),
  rand_generator_(time(NULL)),
  mu_(mu),
  initial_step_count_(step_count),
  current_step_(0),
  graph_step_(graph_step)
{
  num_triangles_ = count_triangles();
  if(0 == initial_step_count_)
  {
    compute_initial_step_count();
  }
}

const std::vector<size_t>& base_task::results() const
{
  return results_;
}

const undirected_graph& base_task::graph() const
{
  return graph_;
}

const std::vector<std::string>& base_task::serialized_graphs() const
{
  return serialized_graphs_;
}

size_t base_task::count_triangles()
{
  size_t num = 0;
  edge_iterator ei, ei_end;
  // iterate over edges
  for(tie(ei, ei_end) = boost::edges(graph_); ei != ei_end; ++ei)
  {
    // get edge vertices
    vertex vs = boost::source(*ei, graph_);
    vertex vt = boost::target(*ei, graph_);
    // get adjacent vertices for them
    adjacency_iterator vsi, vsi_end, vti, vti_end;
    for(tie(vsi, vsi_end) = boost::adjacent_vertices(vs, graph_); vsi != vsi_end; ++vsi)
    {
      for(tie(vti, vti_end) = boost::adjacent_vertices(vt, graph_); vti != vti_end; ++vti)
      {
        if(*vsi == *vti)
        {
          ++num;
        }
      }
    }
  }
  // every triangle is counted trice
  num = num / 3;
  return num;
}

void base_task::perform_randomization()
{
  assert(0 == current_step_);
  results_.push_back(num_triangles_);
  ++current_step_;
  unsigned graph_count = 0;
  if(0 != graph_step_)
  {
    serialized_graphs_.resize(initial_step_count_ / graph_step_ + 1);
    std::string serialized;
    utils::serialize_graph(graph_, serialized);
    std::string compressed;
    utils::compress_string(serialized, compressed);
    serialized_graphs_[graph_count++] = compressed;
  }
  while(current_step_ <= initial_step_count_ || !is_stabilized())
  {
    if(0 == current_step_ % 100000)
    {
      std::cout << "current step = " << current_step_ << std::endl;
    }
    make_randomization_step();
    results_.push_back(num_triangles_);
    if((0 != graph_step_) && (0 == current_step_ % graph_step_))
    {
      std::string serialized;
      utils::serialize_graph(graph_, serialized);
      std::string compressed;
      utils::compress_string(serialized, compressed);
      serialized_graphs_[graph_count++] = compressed;
    }
    ++current_step_;
  }
}

void base_task::make_randomization_step()
{
  //throw new not_implemented_exception(__FUNCTION__" is not implemented.");
  std::cout << "NOT IMPLEMENTED" << std::endl;
}

size_t base_task::compute_initial_step_count()
{
  size_t num_vertices = boost::num_vertices(graph_);
  size_t num_edges = boost::num_edges(graph_);
  size_t num_all_edges = num_vertices * (num_vertices - 1) >> 1;
  initial_step_count_ = num_vertices * mu_ * num_all_edges / num_edges; 
  std::cout << "calculated step count " << initial_step_count_ << std::endl;
}

bool base_task::is_stabilized()
{
  //static std::map<size_t, size_t> stabilization;
  //stabilization[current_step_] = current_step_;

  //static int stabilization_period = 1000;
  //ac::accumulator_set<size_t, ac::features<ac::tag::variance>> acc;
  ////std::for_each(results_.begin(), results_.end(), std::bind<void>(std::ref(acc), _1));
  //for(size_t i = 0; i < results_.size(); ++i)
  //{
  //	acc(results_[i].second);
  //}

  //std::cout << "mean " << ac::mean(acc) << std::endl;
  //std::cout << "variance " << ac::variance(acc) << std::endl;
  //double variance = ;
  //return false;
  return true;
}

bool base_task::check_step(int delta)
{
  if(delta <= 0)
  {
    std::array<long double, 2> probabilities;
    probabilities[0] = exp(mu_ * delta);
    if(std::isinf(probabilities[0]))
    {
      probabilities[0] = 0;
    }
    probabilities[1] = 1.e0 - probabilities[0];
    std::discrete_distribution<int> distribution(probabilities.begin(), probabilities.end());
    if(1 == distribution(rand_generator_))
    {
      return false;
    }
  }
  return true;
}

}
