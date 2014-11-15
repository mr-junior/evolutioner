#include <boost/mpi.hpp>
#include <string>
#include <defs.h>

namespace mpi = boost::mpi;

class main_app
{
  public:
    main_app(int argc, char* argv[]);
    ~main_app();

    void execute();

  private:
    bool parse_command_line();

    void execute_with_single_process();

    void execute_with_multiple_processes();
    void execute_main_process();
    void execute_secondary_process();

		void distribute_data();
		void receive_data();
    void collect_results();
    void finalize();
    void read_config_file(); // for future
		void load_graph_data();
		void load_mu_data();
    void prepare_output_directory();
    void write_output(double mu, const std::vector<std::pair<size_t, double>>& result) const;
		void write_output(double mu, const std::vector<graph_randomization::undirected_graph>& result) const;

		struct graph_data
		{
			graph_randomization::undirected_graph graph_;
			size_t vertex_count_;
			double probability_;
		};

  private:
    int argc_;
    char** argv_;
    std::string type_;
    mpi::environment env_;
    mpi::communicator world_;
    size_t rank_;
    size_t size_;
		std::map<size_t, size_t> process_rank_to_mu_count_;
    std::string output_directory_;
    std::string graph_file_name_;
    std::string mu_file_name_;
		graph_data gr_data_;
		std::vector<double> mu_values_;
    size_t step_count_;
    size_t pass_count_;
		size_t graph_step_;
};
