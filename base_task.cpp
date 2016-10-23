#include "base_task.h"
#include "utils.h"

#include <future>
#include <queue>

namespace gr
{

base_task::base_task(std::shared_ptr<graph_data> graph_data, double mu, size_t step_count, size_t graph_step, const std::string& output_dir) :
  gr_data_(graph_data),
  rand_generator_(time(NULL)),
  mu_(mu),
  initial_step_count_(step_count),
  current_step_(0),
  graph_step_(graph_step),
  pool_(10),
  output_dir_(output_dir)
{
  num_triangles_ = count_triangles();
  if(0 == initial_step_count_)
  {
    compute_initial_step_count();
  }
}

const undirected_graph& base_task::graph() const
{
  return gr_data_->graph_;
}

size_t base_task::count_triangles()
{
  size_t num = 0;
  edge_iterator ei, ei_end;
  // iterate over edges
  std::tie(ei, ei_end) = boost::edges(graph());
  std::vector<edge> edges(ei, ei_end);
#pragma omp parallel for
  for(std::size_t i = 0; i < edges.size(); ++i)
  {
    // get edge vertices
    edge& e = edges[i];
    vertex vs = boost::source(e, graph());
    vertex vt = boost::target(e, graph());
    // get adjacent vertices for them
    adjacency_iterator vsi, vsi_end, vti, vti_end;
    for(tie(vsi, vsi_end) = boost::adjacent_vertices(vs, graph()); vsi != vsi_end; ++vsi)
    {
      for(tie(vti, vti_end) = boost::adjacent_vertices(vt, graph()); vti != vti_end; ++vti)
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

  std::size_t vertices = boost::num_vertices(graph());
  std::stringstream file_name;
  std::string props = gr_data_->get_description();
  file_name << (output_dir_.empty() ? "" : output_dir_ + "/") <<"N" << vertices;
  if(!props.empty())
  {
    file_name << "_" << props;
  }
  file_name << "_u" << mu_ << "_T";
  fs::create_directory(file_name.str());
  file_name << "/N" << vertices;
  if(!props.empty())
  {
    file_name << "_" << props;
  }
  file_name << "_u" << mu_;
  file_name << ".txt";

  std::ofstream output;
  output.open(file_name.str());
  if(!output.is_open())
  {
    std::cerr << "Failed to open/create output file." << std::endl;
    return;
  }
  output << vertices << " " << gr_data_->get_prop_value("p") << " " << mu_ << std::endl;

  output << current_step_ << " " << num_triangles_ << "\n";
  std::string net_prefix;
  if(0 != graph_step_)
  {
    std::stringstream file_name;
    file_name << (output_dir_.empty() ? "" : output_dir_ + "/") <<"N" << vertices;
    if(!props.empty())
    {
      file_name << "_" << props;
    }
    file_name << "_u" << mu_ << "_T";
    fs::create_directory(file_name.str());
    file_name << "/graphs";
    fs::create_directory(file_name.str());
    file_name << "/final_graph__N" << vertices;
    if(!props.empty())
    {
      file_name << "_" << props;
    }
    file_name << "_u" << mu_ << "_";
    net_prefix = file_name.str();
    file_name << current_step_ << ".txt";
    gr_data_->serialize(file_name.str());
  }
  ++current_step_;
  while(current_step_ <= initial_step_count_ || !is_stabilized())
  {
    if(0 == current_step_ % 100000)
    {
      std::cout << "current step = " << current_step_ << std::endl;
    }
    make_randomization_step();
    //if(0 == current_step_ % 1000)
    {
      output << current_step_ << " " << num_triangles_ << "\n";
    }
    if((0 != graph_step_) && (0 == current_step_ % graph_step_))
    {
      std::string file_name = net_prefix + std::to_string(current_step_) + ".txt";
      gr_data_->serialize(file_name);
    }
    ++current_step_;
  }
  output.close();
}

void base_task::make_randomization_step()
{
  //throw new not_implemented_exception(__FUNCTION__" is not implemented.");
  std::cout << "NOT IMPLEMENTED" << std::endl;
}

size_t base_task::compute_initial_step_count()
{
  size_t num_vertices = boost::num_vertices(graph());
  size_t num_edges = boost::num_edges(graph());
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
  if(delta < 0)
  {
    std::bernoulli_distribution distrib(exp(mu_ * delta));
    return distrib(rand_generator_);
  }
  return true;
}

}
