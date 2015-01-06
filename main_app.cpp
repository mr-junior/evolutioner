#include "main_app.h"
#include "task_factory.h"
#include "defs.h"
#include <fstream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <memory>
#include <system_error>
#include <boost/program_options.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/graph/random.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/graph/copy.hpp>
#include <boost/property_map/property_map.hpp>

enum class message_tag : int
{
  vertex_count = 1,
  probability,
  graph,
  step_count,
  pass_count,
  graph_step,
  mu_values_vector,
  sync,
  task_type,
  results_base = 1000,
  results_graphs = 10000
};

//validator for type field
struct randomization_type
{
  randomization_type(const std::string& value) :
    value_(value)
  {}
  std::string value_;
};

void validate(boost::any& v, std::vector<std::string> const& values, randomization_type* /* target_type */, int)
{
  // Make sure no previous assignment to 'v' was made.
  po::validators::check_first_occurrence(v);

  // Extract the first string from 'values'. If there is more than
  // one string, it's an error, and exception will be thrown.
  const std::string& s = po::validators::get_single_string(values);

  if (s == "random_switch" || s == "fixed_degree")
  {
    v = boost::any(randomization_type(s));
  } 
  else 
  {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
}

main_app::main_app(int argc, char* argv[])
  : argc_(argc)
  , argv_(argv)
  , type_(std::string("random_switch"))
  , env_(argc, argv)
  , step_count_(0)
  , pass_count_(1)
	, graph_step_(0)
{
  rank_ = world_.rank();
  size_ = world_.size();
}

main_app::~main_app()
{
  finalize();
}

bool main_app::parse_command_line()
{
  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "Print help message")
    ("graph_file,g", po::value<std::string>()->required(), "File which contains graph")
    ("mu_file,m", po::value<std::string>()->required(), "File which contains mu sequence or mu range with step")
    ("step_count,c", po::value<size_t>(), "Step count per generation.")
    ("pass_count,p", po::value<size_t>(), "Pass count for each mu value.")
    ("type,t", po::value<randomization_type>(), "Randomization type.")
    ("graph_step,s", po::value<size_t>(), "Step count to dump graph from.");

  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc_, argv_, desc), vm);
    if(vm.count("help"))
    {
      std::cout << "Graph Randomizer v1.0" << std::endl << desc << std::endl;
      return false;
    }
    if(vm.count("graph_file"))
    {
      std::cout << "graph file name = " << vm["graph_file"].as<std::string>() << std::endl;
      graph_file_name_ = vm["graph_file"].as<std::string>();
    }
    if(vm.count("mu_file"))
    {
      std::cout << "mu file name = " << vm["mu_file"].as<std::string>() << std::endl;
      mu_file_name_ = vm["mu_file"].as<std::string>();
    }
    if(vm.count("step_count"))
    {
      std::cout << "step count = " << vm["step_count"].as<size_t>() << std::endl;
      step_count_ = vm["step_count"].as<size_t>();
    }
    if(vm.count("pass_count"))
    {
      std::cout << "pass count = " << vm["pass_count"].as<size_t>() << std::endl;
      pass_count_ = vm["pass_count"].as<size_t>();
    }
    if(vm.count("type"))
    {
      std::cout << "type = " << vm["type"].as<randomization_type>().value_ << std::endl;
      type_ = vm["type"].as<randomization_type>().value_;
    }
    if(vm.count("graph_step"))
    {
      std::cout << "graph_step = " << vm["graph_step"].as<size_t>() << std::endl;
      graph_step_ = vm["graph_step"].as<size_t>();
    }
    po::notify(vm);
  }
  catch(po::error& e)
  {
    std::cerr << "\nError parsing command line: " << e.what() << std::endl << std::endl;
    std::cerr << desc << std::endl;
    return false;
  }
  return true;
}

int main_app::execute()
{
  assert(1 <= size_);
  if(0 == rank_ && !parse_command_line())
  {
    finalize();
    env_.abort(-1);
    return -1;
  }
  world_.barrier();
  return (1 == size_) ?
    execute_with_single_process() :
    execute_with_multiple_processes();
}

int main_app::execute_with_single_process()
{
  //load_graph_data();
  //load_mu_data();
  std::cerr << "NOT IMPLEMENTED YET." << std::endl;
  return -1;
}

int main_app::execute_with_multiple_processes()
{
  return (0 == rank_) ?
    execute_main_process() :
    execute_secondary_process();
}

void main_app::finalize()
{
  // finalizing code here
}


void main_app::load_graph_data()
{
  assert(!graph_file_name_.empty());
  std::ifstream graph_file;
  graph_file.open(graph_file_name_);
  if(!graph_file.is_open())
  {
    std::cerr << "Failed to open file containing graph." << std::endl;
    finalize();
    env_.abort(-1);
  }
  archive::text_iarchive ia(graph_file);
  ia >> gr_data_.vertex_count_ >> gr_data_.probability_;
  //std::vector< boost::property<boost::edge_index_t, std::size_t> > edge_properties;
  //edge_properties.resize(gr_data_.vertex_count_);
  //for(size_t i = 0; i < gr_data_.vertex_count_; ++i)
  //{
  //	edge_properties[i].m_value = i;
  //}
  //using namespace graph_randomization;
  //ugraph_without_edge_indexes graph;
  //serialization::load(ia, graph, 0);
  //std::map<vertex, size_t> vertex_to_index;
  //vertex_iterator vertex_it, vertex_it_end;
  //boost::tie(vertex_it, vertex_it_end) = boost::vertices(graph);
  //for(size_t index = 0; vertex_it != vertex_it_end; ++vertex_it, ++index)
  //{
  //	vertex_to_index[*vertex_it] = index;
  //}
  //std::vector< std::pair<size_t, size_t> > edges;
  //boost::graph_traits<ugraph_without_edge_indexes>::edge_iterator edge_it, edge_it_end;
  //boost::tie(edge_it, edge_it_end) = boost::edges(graph);
  //for(; edge_it != edge_it_end; ++edge_it)
  //{
  //	edge e = *edge_it;
  //	vertex source = boost::source(e, graph);
  //	vertex target = boost::target(e, graph);
  //	edges.push_back(std::make_pair(vertex_to_index[source], vertex_to_index[target]));
  //}
  //gr_data_.graph_ = graph_randomization::undirected_graph(edges.begin(), 
  //																												edges.end(), 
  //																												edge_properties.begin(), 
  //																												gr_data_.vertex_count_);
  serialization::load(ia, gr_data_.graph_, 0);
  graph_file.close();
}

void main_app::load_mu_data()
{
  assert(!mu_file_name_.empty());
  std::ifstream mu_file;
  mu_file.open(mu_file_name_);
  if(!mu_file.is_open())
  {
    std::cerr << "Failed to open file containing mu values." << std::endl;
    finalize();
    env_.abort(-1);
  }
  double mu = 0.0;
  while(mu_file >> mu)
  {
    mu_values_.push_back(mu);
  }
  mu_file.close();
}

void main_app::distribute_data()
{
  size_t mu_count = mu_values_.size();
  size_t mu_per_process = mu_count / (size_ - 1);
  size_t remainder = 0;
  if(0 == mu_per_process)
  {
    mu_per_process = 1;
  }
  else
  {
    remainder = mu_count - mu_per_process * (size_ - 1);
  }
  size_t base_index = 0;
  for(size_t i = 1; i < size_; ++i)
  {
    world_.send(i, static_cast<int>(message_tag::vertex_count), gr_data_.vertex_count_);
    world_.send(i, static_cast<int>(message_tag::probability), gr_data_.probability_);
    world_.send(i, static_cast<int>(message_tag::graph), gr_data_.graph_);
    world_.send(i, static_cast<int>(message_tag::step_count), step_count_);
    world_.send(i, static_cast<int>(message_tag::pass_count), pass_count_);
    world_.send(i, static_cast<int>(message_tag::graph_step), graph_step_);

    size_t mu_per_process_i = mu_per_process;
    if(0 != remainder)
    {
      ++mu_per_process_i;
      --remainder;
    }
    std::vector<double> mu_values_for_process_i(mu_values_.begin() + base_index, mu_values_.begin() + base_index + mu_per_process_i);
    world_.send(i, static_cast<int>(message_tag::mu_values_vector), mu_values_for_process_i);
    process_rank_to_mu_count_[i] = mu_per_process_i;
    base_index += mu_per_process_i;

    world_.send(i, static_cast<int>(message_tag::task_type), type_);
    world_.recv(i, static_cast<int>(message_tag::sync));
  }
}

void main_app::collect_results()
{
  std::vector<std::pair<mpi::request, int>> reqs;
  std::vector<std::pair<mpi::request, int>> graph_reqs;
  size_t mu_count = mu_values_.size();
  reqs.resize(mu_count);
  graph_reqs.resize(mu_count);
  std::vector<std::pair<size_t, double>>* results = new std::vector<std::pair<size_t, double>>[mu_count];
  std::vector<graph_randomization::undirected_graph>* final_graphs = new std::vector<graph_randomization::undirected_graph>[mu_count];
  size_t base_index = 0;
  for(size_t i = 1; i < size_; ++i)
  {
    for(size_t j = 0; j < process_rank_to_mu_count_[i]; ++j)
    {
      reqs[base_index + j].first = world_.irecv(i, static_cast<int>(message_tag::results_base)*i+j, results[base_index + j]);
      reqs[base_index + j].second = base_index + j;
    }
    base_index += process_rank_to_mu_count_[i];
  }
  base_index = 0;
  for(size_t i = 1; i < size_; ++i)
  {
    for(size_t j = 0; j < process_rank_to_mu_count_[i]; ++j)
    {
      graph_reqs[base_index + j].first = world_.irecv(i, static_cast<int>(message_tag::results_graphs)*i+j, final_graphs[base_index + j]);
      graph_reqs[base_index + j].second = base_index + j;
    }
    base_index += process_rank_to_mu_count_[i];
  }
  prepare_output_directory();
  while(!reqs.empty())
  {
    for(int i = reqs.size() - 1; i >= 0; --i)
    {
      if(reqs[i].first.test().is_initialized())
      {
        write_output(mu_values_[reqs[i].second], results[reqs[i].second]);
        reqs.erase(reqs.begin() + i);
      }
    }
    usleep(100);
  }
  delete[] results;
  while(!graph_reqs.empty())
  {
    for(int i = graph_reqs.size() - 1; i >= 0; --i)
    {
      if(graph_reqs[i].first.test().is_initialized())
      {
        write_output(mu_values_[graph_reqs[i].second], final_graphs[graph_reqs[i].second]);
        graph_reqs.erase(graph_reqs.begin() + i);
      }
    }
    usleep(100);
  }
  delete[] final_graphs;
}

int main_app::execute_main_process()
{
  assert(1 < size_);
  std::cout << "[main]: Loading data..." << std::endl;
  load_graph_data();
  load_mu_data();
  std::cout << "[main]: Finished loading data." << std::endl;
  std::cout << "[main]: Distributing data to processes..." << std::endl;
  distribute_data();
  std::cout << "[main]: Finished distributing data to processes." << std::endl;
  std::cout << "[main]: Collecting results..." << std::endl;
  collect_results();
  std::cout << "[main]: Finished collecting results." << std::endl;
  return 0;
}

void main_app::receive_data()
{
  world_.recv(0, static_cast<int>(message_tag::vertex_count), gr_data_.vertex_count_);
  world_.recv(0, static_cast<int>(message_tag::probability), gr_data_.probability_);
  world_.recv(0, static_cast<int>(message_tag::graph), gr_data_.graph_);
  world_.recv(0, static_cast<int>(message_tag::step_count), step_count_);
  world_.recv(0, static_cast<int>(message_tag::pass_count), pass_count_);
  world_.recv(0, static_cast<int>(message_tag::graph_step), graph_step_);
  world_.recv(0, static_cast<int>(message_tag::mu_values_vector), mu_values_);
  world_.recv(0, static_cast<int>(message_tag::task_type), type_);
  world_.send(0, static_cast<int>(message_tag::sync));
}

int main_app::execute_secondary_process()
{
  receive_data();
  size_t mu_count = mu_values_.size();
  mpi::request* reqs = new mpi::request[mu_count];
  mpi::request* graph_reqs = new mpi::request[mu_count];
  for(size_t i = 0; i < mu_count; ++i)
  {
    std::vector<std::pair<size_t, double>> results;
    std::vector<graph_randomization::undirected_graph> graphs;
    for(size_t p = 0; p < pass_count_; ++p)
    {
      std::shared_ptr<graph_randomization::base_task> task = graph_randomization::get_task(gr_data_.graph_, mu_values_[i], step_count_, graph_step_, type_); 
      task->perform_randomization();
      const std::vector<std::pair<size_t, size_t>>& results_for_i = task->results();
      results.resize(results_for_i.size());
      for(size_t k = 0; k < results.size(); ++k)
      {
        results[k].first = k;
      }
      //assert(step_count_ + 1 == results_for_i.size());
      for(size_t j = 0; j < results_for_i.size(); ++j)
      {
        assert(results[j].first == results_for_i[j].first);
        results[j].second += static_cast<double>(results_for_i[j].second);
      }
      //graph.clear();
      //boost::copy_graph(task->graph(), graph);
      graphs = task->graphs();
    }
    for(size_t j = 0; j < results.size(); ++j)
    {
      results[j].second /= pass_count_;
    }
    reqs[i] = world_.isend(0, static_cast<int>(message_tag::results_base)*rank_ + i, results);
    graph_reqs[i] = world_.isend(0, static_cast<int>(message_tag::results_graphs)*rank_ + i, graphs);
  }
  mpi::wait_all(reqs, reqs + mu_count);
  mpi::wait_all(graph_reqs, graph_reqs + mu_count);
  delete[] reqs;
  delete[] graph_reqs;
  return 0;
}

void main_app::prepare_output_directory()
{
  using std::chrono::system_clock;
  std::stringstream output_folder;
  std::time_t tt = system_clock::to_time_t(system_clock::now());
  struct std::tm* ptm = std::localtime(&tt);
  char buf[100];
  strftime(buf, sizeof(buf), "%Y.%m.%d_%H.%M.%S", ptm);
  output_folder << "results_" << buf;//std::put_time(ptm, "%F_%T");
  output_directory_ = output_folder.str();
  fs::create_directory(output_directory_);
}

void main_app::write_output(double mu, const std::vector<std::pair<size_t, double>>& result) const
{
  std::stringstream file_name;
  file_name << (output_directory_.empty() ? "" : output_directory_ + "/") <<"N" << gr_data_.vertex_count_ 
    << "_p" << gr_data_.probability_ << "_u" << mu << "_T";
  //try
  //{
  fs::create_directory(file_name.str());
  //}
  //catch(fs::basic_filesystem_error&)
  //{
  //  std::cerr << "Directory already exists." << std::endl;
  //  return;
  //}
  file_name << "/N" << gr_data_.vertex_count_ << "_p" << gr_data_.probability_ << "_u" << mu << ".txt";

  std::ofstream output;
  output.open(file_name.str());
  if(!output.is_open())
  {
    std::cerr << "Failed to open/create output file." << std::endl;
    return;
  }
  output << gr_data_.vertex_count_ << " " << gr_data_.probability_ << " " << mu << std::endl;
  for(size_t i = 0; i < result.size(); ++i)
  {
    output << result[i].first << " " << result[i].second << std::endl;
  }
  output.close();
}

void main_app::write_output(double mu, const std::vector<graph_randomization::undirected_graph>& result) const
{
  for(size_t i = 0; i < result.size(); ++i)
  {
    std::stringstream file_name;
    file_name << (output_directory_.empty() ? "" : output_directory_ + "/") <<"N" << gr_data_.vertex_count_ 
      << "_p" << gr_data_.probability_ << "_u" << mu << "_T";
    fs::create_directory(file_name.str());
    file_name << "/final_graph__N" << gr_data_.vertex_count_ << "_p" << gr_data_.probability_ << "_u" << mu << "_" << i * graph_step_ << ".txt";

    std::ofstream file;
    file.open(file_name.str());
    if(!file.is_open())
    {
      std::cerr << "Failed to open/create output file." << std::endl;
      return;
    }
    boost::archive::text_oarchive oa(file);
    oa << gr_data_.vertex_count_ << gr_data_.probability_;
    boost::serialization::save(oa, result[i], 0);

    file.close();
  }
}
