#pragma once

#include "defs.h"

namespace gr
{

namespace utils
{

void copy_graph(const undirected_graph& input, undirected_graph& output);

void serialize_graph(const undirected_graph& graph, std::string& str);

void deserialize_graph(const std::string& str, undirected_graph& graph);

void compress_string(const std::string& input, std::string& output);

void decompress_string(const std::string& input, std::string& output);

}

}
