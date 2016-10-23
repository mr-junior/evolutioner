#pragma once

#include "base_task.h"

namespace gr
{

class fixed_degree_task : public base_task
{
public:
  fixed_degree_task(std::shared_ptr<graph_data> graph_data, double mu, int step_count, size_t graph_step, const std::string& output_dir);
  ~fixed_degree_task();
  virtual void make_randomization_step();
};

}
