#pragma once

#include "base_task.h"

namespace graph_randomization
{

class random_switch_task : public base_task
{
public:
  random_switch_task(const undirected_graph& graph, double mu, size_t step_count, size_t graph_step);
  ~random_switch_task();
  virtual void make_randomization_step();

private:
  boost::variate_generator<random_generator, std::uniform_int_distribution<>>* var_generator_;
  std::vector<std::pair<vertex, vertex>> non_existing_edges_;
};

}
