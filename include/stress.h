#pragma once

#include <sys/resource.h>

#include "graph.h"
#include "common.h"

uint32_t stress_evaluate_runtime_complexity_helper(GraphBuilder, TraversalAlgo_t, ChronoId, char*, uint32_t);
struct rusage differential_traversal_measurement(GraphHandle, PersistedGraph, TraversalAlgo_t);
void stress_runtime_complexity_all_algo();
void stress_profile_algo(uint32_t, PersistedGraph, GraphBuilder, TraversalAlgo_t);
void stress_profile_algo_on_all_graph_types(uint32_t, TraversalAlgo_t);

