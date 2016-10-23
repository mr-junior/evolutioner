#pragma once

#include "defs.h"
#include "thread_pool.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/graph/random.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <fstream>
#include <random>
#include <thread>
#include <vector>

namespace gr
{

class base_task
{
public:
  base_task(std::shared_ptr<graph_data> graph, double mu, size_t step_count, size_t graph_step, const std::string& output_dir);
  virtual void perform_randomization();
  const undirected_graph& graph() const;

private:
  virtual void make_randomization_step() = 0;

protected:
  virtual bool is_stabilized();
  virtual bool check_step(int delta);
  size_t count_triangles();
  size_t compute_initial_step_count();

protected:
  std::shared_ptr<graph_data> gr_data_;
  size_t num_triangles_;
  random_generator rand_generator_;
  double mu_;
  size_t initial_step_count_;
  size_t current_step_;
  size_t graph_step_;
  thread_pool pool_;
  std::string output_dir_;
};

}
