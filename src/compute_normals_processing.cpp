/*
Copyright (c) 2016 Ravi Peters

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// #define VERBOSEPRINT 1;
// #define WITH_OPENMP 1;

#include <iostream>
#include <limits>

// OpenMP
#ifdef WITH_OPENMP
#include <omp.h>
#endif

// Vrui
#include <vrui/Geometry/ComponentArray.h>
#include <vrui/Math/Math.h>
#include <vrui/Geometry/PCACalculator.h>
#ifndef __MINGW32__
#include <vrui/Misc/Timer.h>
#endif
// kdtree2
#include <kdtree2/kdtree2.hpp>

// typedefs
#include "compute_normals_processing.h"





//==============================
//   COMPUTE NORMALS
//==============================


Vector estimate_normal(Point &p, kdtree2::KDTree* kd_tree, int k)
{
   kdtree2::KDTreeResultVector result;
   kd_tree->n_nearest(p, k + 1, result);

   Geometry::PCACalculator<3> PCACalc;
   for (int i = 0; i < k + 1; i++)
      PCACalc.accumulatePoint(kd_tree->the_data[result[i].idx]);

   double eigen_values[3];
   PCACalc.calcCovariance();
   PCACalc.calcEigenvalues(eigen_values);
   return PCACalc.calcEigenvector(eigen_values[2]);
}

VectorList estimate_normals(PointList &points, kdtree2::KDTree* kd_tree, int k)
{
   VectorList normals(points.size());

#pragma omp parallel for
   for (int i = 0; i < points.size(); i++)
      normals[i] = estimate_normal(points[i], kd_tree, k);

   return normals;
}

void compute_normals(normals_parameters &input_parameters, PointList &coords, VectorList &normals)
{

#ifndef __MINGW32__
   Misc::Timer t0;
#endif
   kdtree2::KDTree* kd_tree;
   kd_tree = new kdtree2::KDTree(coords, input_parameters.kd_tree_reorder);
   kd_tree->sort_results = false;
#ifndef __MINGW32__
   t0.elapse();
   std::cout << "Constructed kd-tree in " << t0.getTime()*1000.0 << " ms" << std::endl;
#endif

   // omp_set_num_threads(1);

   {
      normals = estimate_normals(coords, kd_tree, input_parameters.k);
#ifndef __MINGW32__
      t0.elapse();
      std::cout << "Done estimating normals, took " << t0.getTime()*1000.0 << " ms" << std::endl;
#endif
   }
}

