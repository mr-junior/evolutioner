#ifndef _BASE_TASK_H_
#define _BASE_TASK_H_

#include "defs.h"
#include <fstream>
#include <random>
#include <thread>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/graph/random.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace graph_randomization
{

class base_task
{
public:
  base_task(const undirected_graph& graph, double mu, size_t step_count, size_t graph_step);
  virtual void perform_randomization();
  const std::vector<std::pair<size_t, size_t>>& results() const;
	const undirected_graph& graph() const;
	const std::vector<undirected_graph>& graphs() const;

private:
  virtual void make_randomization_step() = 0;

protected:
  virtual bool is_stabilized();
  virtual bool check_step(int delta);
  size_t count_triangles();
	size_t compute_initial_step_count();

protected:
  undirected_graph graph_;
  size_t num_triangles_;
  random_generator rand_generator_;
  size_t num_vertices_;
  double mu_;
  double probability_;
  std::vector<std::pair<size_t, size_t>> results_;
  size_t initial_step_count_;
	size_t current_step_;
	size_t graph_step_;
	std::vector<undirected_graph> graphs_;
};

} // end namespace graph_randomization

#endif // _BASE_TASK_H_
