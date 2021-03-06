#include <iostream>
#include <iomanip>
#include <cassert>

// First step in porting mc-mini from Eigen2 to Eigen3.
// http://eigen.tuxfamily.org/dox/Eigen2ToEigen3.html

#define EIGEN2_SUPPORT_STAGE10_FULL_EIGEN2_API true

#include <Eigen/OrderingMethods>
#include <Eigen/SparseCholesky>
#include <Eigen/SparseQR>
#include <Eigen/Sparse>
#include <Eigen/Dense>

#include "matrixForms/sparseForms.h"
#include "matrixForms/denseForms.h"
#include "geometry/geometry.h"
#include "geometry/domain.h"
#include "problem/problem.h"
#include "output/output.h"
#include "parser/parser.h"


using namespace Eigen;
using namespace std;

int main(int argc, char ** argv) {

  if (argc == 1) {
    cerr << "usage: " << string{argv[0]} << " <parameter file>." << endl << endl;
    exit (-1);
  }

  setNbThreads(16);

  ParamParser parser(string{argv[1]});
  GeometryStructure geometry (parser);
  //Cell-centered domain 
  Domain grid(geometry);
  ProblemStructure  problem  (parser, geometry, grid);
  OutputStructure   output   (parser, geometry, problem);

  problem.initializeProblem();
  
  problem.updateForcingTerms();
  problem.solveStokes();

  problem.recalculateTimestep();
  
  do {    
    output.writeHDF5File (problem.getTimestepNumber());
    cout << "<Timestep: " << problem.getTimestepNumber() << "; t=" << problem.getTime() << ">" << endl << endl;
    problem.updateForcingTerms();
    problem.solveStokes();
    problem.recalculateTimestep();
    problem.solveAdvectionDiffusion();

  } while (problem.advanceTimestep());

  output.writeHDF5File (problem.getTimestepNumber());
  cerr << "Timestep: " << problem.getTimestepNumber() << "; t = " << problem.getTime() << endl;

  output.writeHDF5File();
  
  return 0;
}
