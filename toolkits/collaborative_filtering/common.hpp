#ifndef _COMMON_H__
#define _COMMON_H__

/**
 * @file
 * @author  Danny Bickson, based on code by Aapo Kyrola
 * @version 1.0
 *
 * @section LICENSE
 *
 * Copyright [2012] [Carnegie Mellon University]
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
 
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <errno.h>
#include <string>

#include "util.hpp"
#include "graphchi_basic_includes.hpp"
#include "api/vertex_aggregator.hpp"
#include "preprocessing/sharder.hpp"
#include "../../example_apps/matrix_factorization/matrixmarket/mmio.h"
#include "../../example_apps/matrix_factorization/matrixmarket/mmio.c"

#ifndef NLATENT
#define NLATENT 20   // Dimension of the latent factors. You can specify this in compile time as well (in make).
#endif

using namespace graphchi;

double minval = -1e100;
double maxval = 1e100;
std::string training;
std::string validation;
std::string test;
uint M, N, K;
size_t L;
uint Me, Ne, Le;
double globalMean = 0;
double rmse=0.0;
bool load_factors_from_file = false;
int unittest = 0;
int niters = 10;
int halt_on_rmse_increase = 0;

void parse_command_line_args(){
  /* Basic arguments for application. NOTE: File will be automatically 'sharded'. */
  unittest = get_option_int("unittest", 0);
  niters    = get_option_int("max_iter", 6);  // Number of iterations
  if (unittest > 0)
    training = get_option_string("training", "");    // Base filename
  else training = get_option_string("training");
  validation = get_option_string("validation", "");
  test = get_option_string("test", "");

  if (validation == "")
    validation += training + "e";  
  if (test == "")
    test += training + "t";

  maxval        = get_option_float("maxval", 1e100);
  minval        = get_option_float("minval", -1e100);
  bool quiet    = get_option_int("quiet", 0);
  if (quiet)
    global_logger().set_log_level(LOG_ERROR);
  halt_on_rmse_increase = get_option_int("halt_on_rmse_increase", 0);

  load_factors_from_file = get_option_int("load_factors_from_file", 0);
}

template<typename T>
void set_engine_flags(T & pengine){
  pengine.set_disable_vertexdata_storage();  
  pengine.set_enable_deterministic_parallelism(false);
  pengine.set_modifies_inedges(false);
  pengine.set_modifies_outedges(false);
  pengine.set_preload_commit(false);
}

void print_copyright(){
  logstream(LOG_WARNING)<<"GraphChi Collaborative filtering library is written by Danny Bickson (c). Send any "
    " comments or bug reports to danny.bickson@gmail.com " << std::endl;
}

void print_config(){
 std::cout<<"[feature_width] => [" << NLATENT << "]" << std::endl;
 std::cout<<"[users] => [" << M << "]" << std::endl;
 std::cout<<"[movies] => [" << N << "]" <<std::endl;
 std::cout<<"[training_ratings] => [" << L << "]" << std::endl;
 std::cout<<"[number_of_threads] => [" << number_of_omp_threads() << "]" <<std::endl;
 std::cout<<"[membudget_Mb] => [" << get_option_int("membudget_mb") << "]" <<std::endl; 
}

template<typename T>
void init_feature_vectors(uint size, T& latent_factors_inmem, bool randomize = true){
  assert(size > 0);

  srand48(time(NULL));
  latent_factors_inmem.resize(size); // Initialize in-memory vertices.
  if (!randomize)
    return;

#pragma omp parallel for
 for (uint i=0; i < size; i++){
    for (uint j=0; j<NLATENT; j++)
      latent_factors_inmem[i].pvec[j] = drand48();
  } 
}
#endif //_COMMON_H__