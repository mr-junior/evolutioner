#pragma once

#include "base_task.h"

namespace gr
{

class random_switch_task : public base_task
{
public:
  random_switch_task(std::shared_ptr<graph_data> graph_data, double mu, size_t step_count, size_t graph_step, const std::string& output_dir);
  ~random_switch_task();
  virtual void make_randomization_step();

private:
  boost::variate_generator<random_generator, std::uniform_int_distribution<>>* var_generator_;
  std::vector<std::pair<vertex, vertex>> non_existing_edges_;
};

}
