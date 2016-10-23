#pragma once

#include "base_task.h"
#include "fixed_degree_task.h"
#include "random_switch_task.h"
#include <memory>

namespace gr
{

std::shared_ptr<base_task> get_task(std::shared_ptr<graph_data> graph_data, double mu, size_t step_count, size_t graph_step, const std::string& type, const std::string& output_dir);

}
