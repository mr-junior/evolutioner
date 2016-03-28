#include "main_app.h"
#include "task_factory.h"
#include "utils.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/graph/random.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/serialization/utility.hpp>

#include <chrono>
#include <ctime>
#include <fstream>
#include <future>
#include <iomanip>
#include <memory>
#include <system_error>
#include <vector>

namespace
{

enum class message_tag : int
{
  mu_values_vector,
  results_directory,
  sync
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

}

namespace gr
{

main_app::main_app(int argc, char** argv) :
  env_(argc, argv),
  type_(std::string("random_switch")),
  step_count_(0),
  pass_count_(1),
  graph_step_(0)
{
  parse_command_line(argc, argv);
}

main_app::~main_app()
{
}

bool main_app::parse_command_line(int argc, char** argv)
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
    po::store(po::parse_command_line(argc, argv, desc), vm);
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
  return (1 == world_.size()) ?
    execute_with_single_process() :
    execute_with_multiple_processes();
}

int main_app::execute_with_single_process()
{
  std::cout << "[rank-0]: Loading data..." << std::endl;
  load_graph_data();
  load_mu_data();
  std::cout << "[rank-0]: Finished loading data." << std::endl;
  return execute_working_process();
}

int main_app::execute_with_multiple_processes()
{
  return (0 == world_.rank()) ?
    execute_main_process() :
    execute_secondary_process();
}

void main_app::load_graph_data()
{
  assert(!graph_file_name_.empty());
  std::ifstream graph_file;
  graph_file.open(graph_file_name_);
  if(!graph_file.is_open())
  {
    std::cerr << "Failed to open file containing graph." << std::endl;
    env_.abort(-1);
  }
  archive::text_iarchive ia(graph_file);
  ia >> gr_data_.vertex_count_ >> gr_data_.probability_;
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
  std::size_t mu_count = mu_values_.size();
  std::size_t mu_per_process = mu_count / (world_.size() - 1);
  for(std::size_t i = 1; i < world_.size() && !mu_values_.empty(); ++i)
  {
    // send output directory name
    world_.send(i, static_cast<int>(message_tag::results_directory), output_directory_);

    // send mu values for calculation
    std::vector<double> mu_values(mu_values_.begin(), mu_values_.begin() + mu_per_process);
    world_.send(i, static_cast<int>(message_tag::mu_values_vector), mu_values);
    mu_values_.erase(mu_values_.begin(), mu_values_.begin() + mu_per_process);
  }
}

int main_app::execute_main_process()
{
  assert(1 < world_.size());
  std::cout << "[rank-0]: Loading data..." << std::endl;
  load_graph_data();
  load_mu_data();
  std::cout << "[rank-0]: Finished loading data." << std::endl;
  std::cout << "[rank-0]: Distributing data to processes..." << std::endl;
  prepare_output_directory();
  distribute_data();
  std::cout << "[rank-0]: Finished distributing data to processes." << std::endl;
  return execute_working_process();
}

int main_app::execute_secondary_process()
{
  assert(1 < world_.size());
  std::cout << "[rank-" << world_.rank() << "]: Loading data..." << std::endl;
  load_graph_data();
  std::cout << "[rank-" << world_.rank() << "]: Finished loading data." << std::endl;
  receive_data();
  execute_working_process();
}

void main_app::receive_data()
{
  world_.recv(0, static_cast<int>(message_tag::mu_values_vector), mu_values_);
  world_.recv(0, static_cast<int>(message_tag::results_directory), output_directory_);
}

int main_app::execute_working_process()
{
  if(!mu_values_.empty())
  {
    std::cout << "[rank-" << world_.rank() << "]: Generating trajectories..." << std::endl;
    size_t mu_count = mu_values_.size();
    std::cout << mu_count << std::endl;
    for(size_t i = 0; i < mu_count; ++i)
    {
      std::vector<double> results;
      std::vector<std::string> serialized_graphs;
      for(size_t p = 0; p < pass_count_; ++p)
      {
        std::shared_ptr<gr::base_task> task = gr::get_task(gr_data_.graph_, mu_values_[i], step_count_, graph_step_, type_);
        task->perform_randomization();
        const std::vector<size_t>& results_for_i = task->results();
        results.resize(results_for_i.size());
        //assert(step_count_ + 1 == results_for_i.size());
        for(size_t j = 0; j < results_for_i.size(); ++j)
        {
          results[j] += static_cast<double>(results_for_i[j]);
        }
        serialized_graphs = task->serialized_graphs();
      }
      for(size_t j = 0; j < results.size(); ++j)
      {
        results[j] /= pass_count_;
      }
      write_output(mu_values_[i], results);
      write_output(mu_values_[i], serialized_graphs);
    }
    std::cout << "[rank-" << world_.rank() << "]: Finished generating trajectories." << std::endl;
  }

  std::vector<mpi::request> waiting_for;
  for(std::size_t i = 0; i < world_.size(); ++i)
  {
    if(i != world_.rank())
    {
      world_.isend(i, static_cast<int>(message_tag::sync));
      waiting_for.push_back(world_.irecv(i, static_cast<int>(message_tag::sync)));
    }
  }
  std::chrono::milliseconds span(1500);
  while(!mpi::test_all(waiting_for.begin(), waiting_for.end()))
  {
    std::this_thread::sleep_for(span);
  }
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

void main_app::write_output(double mu, const std::vector<double>& result) const
{
  std::stringstream file_name;
  file_name << (output_directory_.empty() ? "" : output_directory_ + "/") <<"N" << gr_data_.vertex_count_ 
    << "_p" << gr_data_.probability_ << "_u" << mu << "_T";
  fs::create_directory(file_name.str());
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
    output << i << " " << result[i] << "\n";
  }
  output.close();
}

void main_app::write_output(double mu, const std::vector<std::string>& result) const
{
  for(size_t i = 0; i < result.size(); ++i)
  {
    std::stringstream file_name;
    file_name << (output_directory_.empty() ? "" : output_directory_ + "/") <<"N" << gr_data_.vertex_count_ 
      << "_p" << gr_data_.probability_ << "_u" << mu << "_T";
    fs::create_directory(file_name.str());
    file_name << "/graphs";
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
    undirected_graph graph;
    utils::deserialize_graph(result[i], graph);
    boost::serialization::save(oa, graph, 0);

    file.close();
  }
}

}
