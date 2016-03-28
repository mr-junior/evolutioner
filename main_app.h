#pragma once

#include "defs.h"

#include <string>
#include <map>

namespace gr
{

class main_app
{
public:
  main_app(int argc, char** argv);
  ~main_app();

  int execute();

private:
  bool parse_command_line(int argc, char** argv);

  int execute_with_single_process();

  int execute_with_multiple_processes();
  int execute_main_process();
  int execute_secondary_process();

  int execute_working_process();

  void distribute_data();
  void receive_data();
  void load_graph_data();
  void load_mu_data();
  void prepare_output_directory();
  void write_output(double mu, const std::vector<double>& result) const;
  void write_output(double mu, const std::vector<std::string>& result) const;

  struct graph_data
  {
    undirected_graph graph_;
    size_t vertex_count_;
    double probability_;
  };

private:
  mpi::environment env_;
  mpi::communicator world_;
  std::string type_;
  std::string output_directory_;
  std::string graph_file_name_;
  std::string mu_file_name_;
  graph_data gr_data_;
  std::vector<double> mu_values_;
  std::size_t step_count_;
  std::size_t pass_count_;
  std::size_t graph_step_;
};

}
