#ifndef _TASK_FACTORY_H_
#define _TASK_FACTORY_H_

#include "base_task.h"
#include "fixed_degree_task.h"
#include "random_switch_task.h"
#include <memory>

namespace graph_randomization
{

std::shared_ptr<base_task> get_task(const undirected_graph& graph, double mu, size_t step_count, size_t graph_step, const std::string& type);

}

#endif //_TASK_FACTORY_H_
