#pragma once

#include "base_task.h"

namespace graph_randomization
{

class fixed_degree_task : public base_task
{
public:
  fixed_degree_task(const undirected_graph& graph, double mu, int step_count, size_t graph_step);
  ~fixed_degree_task();
  virtual void make_randomization_step();
};

}
