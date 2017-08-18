#include <stress.h>

#include <stdlib.h>
#include <string.h>

#include <logger.h>
#include <common.h>
#include <util.h>
#include <graph.h>
#include <special_traversals.h>

struct rusage differential_traversal_measurement(
    GraphHandle graph, PersistedGraph graph_buf, TraversalAlgo_t traversal_algo) {
  const uint32_t repetitions = 16;
  struct rusage final_chrono;

  clear_chrono_by_id(__CHRONO_FIRST__);
  CHRONO_START(__CHRONO_FIRST__);
  for(uint32_t repeat=0; repeat<repetitions; ++repeat) {
    restore_graph_from_buffer_no_offset_adjust(graph_buf, graph);
    traversal_algo(graph.root, NULL, nop_visitor);
  }
  CHRONO_STOP(__CHRONO_FIRST__);

  clear_chrono_by_id(__CHRONO_SECOND__);
  CHRONO_START(__CHRONO_SECOND__);
  for(uint32_t repeat=0; repeat<repetitions; ++repeat) {
    restore_graph_from_buffer_no_offset_adjust(graph_buf, graph);
  }
  CHRONO_STOP(__CHRONO_SECOND__);

  diff_chrono_by_id(&final_chrono, __CHRONO_SECOND__, __CHRONO_FIRST__);
  return final_chrono;
}

uint32_t stress_evaluate_runtime_complexity_helper(
    GraphBuilder graph_builder, TraversalAlgo_t traversal_algo, 
    ChronoId chrono, char* buffer, uint32_t buflen) {
  uint32_t init_buflen = buflen;
  
  for(uint32_t size=8192; size<512*1024; size+=32*1024) {
    GraphHandle graph = graph_builder(size);
    PersistedGraph graph_buf = persist_graph_to_new_buffer(graph);

    struct rusage measurement = differential_traversal_measurement(graph, graph_buf, traversal_algo);
    uint32_t written = chrono_to_csv_by_obj(
      buffer, buflen, ChronoId_to_string(chrono), &measurement);

    TEST_ASSERT(written < buflen, "Buffer too short to collect stats");
    LOG_TRACE("For %u took : %s", size, buffer);
    buffer += written;
    buflen -= written;

    free_graph(graph);
    free_persisted_graph(graph_buf);
  }

  // the number of chars written (excluding terminating '\0')
  return init_buflen - buflen;
}

void stress_profile_algo(uint32_t size, PersistedGraph graph_buf, GraphBuilder graph_builder, TraversalAlgo_t traversal_algo) {
  const uint32_t repetitions = 16;
  GraphHandle graph = graph_builder(size);
  persist_graph_to_old_buffer(&graph_buf, graph);

  for(uint32_t repeat=0; repeat<repetitions; ++repeat) {
    if (repeat)
      restore_graph_from_buffer_no_offset_adjust(graph_buf, graph);
    traversal_algo(graph.root, NULL, nop_visitor);
  }
  free_graph(graph);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void stress_profile_algo_on_all_graph_types(uint32_t size, const char* algo_name) {
  TraversalAlgo_t traversal_algo = get_function_by_name(algo_name);
  if (traversal_algo == NULL) {
    LOG_WARN("Could not find algorithm name : '%s'", algo_name);
    return;
  }
  PersistedGraph graph_buf = build_persist_graph_buffer(size);

  stress_profile_algo(size, graph_buf, build_graph_dag, traversal_algo);
  stress_profile_algo(size, graph_buf, build_graph_with_undirected_cycles, traversal_algo);
  stress_profile_algo(size, graph_buf, build_graph_with_cycles, traversal_algo);

  free_persisted_graph(graph_buf);
}

void stress_runtime_complexity_all_algo(const char* report_name) {
  char* buffer = malloc(1024*1024);
  uint32_t written, bufpos=0, buflen=1024*1024;

  report_name = report_name ? report_name : "complexity_runtime.log";
  FILE *report = fopen(report_name, "w");
  ASSERT(report, "Failed to open for write : %s", report_name);

  written = chrono_header_to_csv(buffer, buflen);
  bufpos += written;
  buflen -= written;

/* destructive_pointer_reversal_traversal */

  written = stress_evaluate_runtime_complexity_helper(
    build_graph_dag, destructive_pointer_reversal_traversal, 
    DESTRUCT_PTR_REVERSAL_DAG, buffer + bufpos, buflen);

  bufpos += written;
  buflen -= written;

  written = stress_evaluate_runtime_complexity_helper(
    build_graph_with_undirected_cycles, destructive_pointer_reversal_traversal, 
    DESTRUCT_PTR_REVERSAL_UCYCLE, buffer + bufpos, buflen);

  bufpos += written;
  buflen -= written;

  written = stress_evaluate_runtime_complexity_helper(
    build_graph_with_cycles, destructive_pointer_reversal_traversal, 
    DESTRUCT_PTR_REVERSAL_DCYCLE, buffer + bufpos, buflen);

  bufpos += written;
  buflen -= written;

/* destructive_pointer_back_and_forth_traversal */

  written = stress_evaluate_runtime_complexity_helper(
    build_graph_dag, destructive_pointer_back_and_forth_traversal, 
    DESTRUCT_BACK_FORTH_DAG, buffer + bufpos, buflen);

  bufpos += written;
  buflen -= written;

  written = stress_evaluate_runtime_complexity_helper(
    build_graph_with_undirected_cycles, destructive_pointer_back_and_forth_traversal, 
    DESTRUCT_BACK_FORTH_UCYCLE, buffer + bufpos, buflen);

  bufpos += written;
  buflen -= written;

  written = stress_evaluate_runtime_complexity_helper(
    build_graph_with_cycles, destructive_pointer_back_and_forth_traversal, 
    DESTRUCT_BACK_FORTH_DCYCLE, buffer + bufpos, buflen);

  bufpos += written;
  buflen -= written;

/* pointer_reversal_traversal */

  written = stress_evaluate_runtime_complexity_helper(
    build_graph_dag, pointer_reversal_traversal, 
    PTR_REVERSAL_DAG, buffer + bufpos, buflen);

  bufpos += written;
  buflen -= written;

  written = stress_evaluate_runtime_complexity_helper(
    build_graph_with_undirected_cycles, pointer_reversal_traversal, 
    PTR_REVERSAL_UCYCLE, buffer + bufpos, buflen);

  bufpos += written;
  buflen -= written;

  written = stress_evaluate_runtime_complexity_helper(
    build_graph_with_cycles, pointer_reversal_traversal, 
    PTR_REVERSAL_DCYCLE, buffer + bufpos, buflen);

  bufpos += written;
  buflen -= written;

/* destructive_std_depth_first_traversal */

  written = stress_evaluate_runtime_complexity_helper(
    build_graph_dag, destructive_std_depth_first_traversal, 
    STD_DEPTH_FIRST_DAG, buffer + bufpos, buflen);

  bufpos += written;
  buflen -= written;

  written = stress_evaluate_runtime_complexity_helper(
    build_graph_with_undirected_cycles, destructive_std_depth_first_traversal, 
    STD_DEPTH_FIRST_UCYCLE, buffer + bufpos, buflen);

  bufpos += written;
  buflen -= written;

  written = stress_evaluate_runtime_complexity_helper(
    build_graph_with_cycles, destructive_std_depth_first_traversal, 
    STD_DEPTH_FIRST_DCYCLE, buffer + bufpos, buflen);

  bufpos += written;
  buflen -= written;

  written = fwrite(buffer, sizeof(char), bufpos, report);
  ASSERT(written == bufpos, "Failed to write report : %s", report_name);
  free(buffer);
  fclose(report);
}

