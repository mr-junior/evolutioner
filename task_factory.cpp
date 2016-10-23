#include "task_factory.h"

namespace gr
{

std::shared_ptr<base_task> get_task(std::shared_ptr<graph_data> graph_data, double mu, size_t step_count, size_t graph_step, const std::string& type, const std::string& output_dir)
{
  if(std::string("random_switch") == type)
  {
    return std::make_shared<random_switch_task>(graph_data, mu, step_count, graph_step, output_dir);
  }
  else if(std::string("fixed_degree") == type)
  {
    return std::make_shared<fixed_degree_task>(graph_data, mu, step_count, graph_step, output_dir);
  }
  else
  {
    std::cerr << "Warning: returning null task." << std::endl;
    return std::shared_ptr<base_task>();
  }
}

}
