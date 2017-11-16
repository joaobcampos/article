/******************************************************************************
 * Author:   Laurent Kneip                                                    *
 * Contact:  kneip.laurent@gmail.com                                          *
 * License:  Copyright (c) 2013 Laurent Kneip, ANU. All rights reserved.      *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions         *
 * are met:                                                                   *
 * * Redistributions of source code must retain the above copyright           *
 *   notice, this list of conditions and the following disclaimer.            *
 * * Redistributions in binary form must reproduce the above copyright        *
 *   notice, this list of conditions and the following disclaimer in the      *
 *   documentation and/or other materials provided with the distribution.     *
 * * Neither the name of ANU nor the names of its contributors may be         *
 *   used to endorse or promote products derived from this software without   *
 *   specific prior written permission.                                       *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"*
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE  *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE *
 * ARE DISCLAIMED. IN NO EVENT SHALL ANU OR THE CONTRIBUTORS BE LIABLE        *
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL *
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR *
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER *
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT         *
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY  *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF     *
 * SUCH DAMAGE.                                                               *
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <opengv/relative_pose/methods.hpp>
#include <opengv/relative_pose/NoncentralRelativeAdapter.hpp>
#include <sstream>
#include <fstream>

#include "container.h"
#include "random_generators.hpp"
#include "experiment_helpers.hpp"
#include "time_measurement.hpp"


using namespace std;
using namespace Eigen;
using namespace opengv;

int main( int argc, char** argv )
{
  // initialize random seed
  initializeRandomSeed();
  int n_experiments = 4;
  int noise_levels = 3;
  //set experiment parameters
  double outlierFraction = 0.0;
  size_t numberPoints = 100;
  int numberCameras = 4;

  std::vector<Container> information_list17pt;
  std::vector<Container> information_listNL;
  std::vector<Container> information_ge;
  std::vector<Container> information_riemann;
  for(int index = 0; index < noise_levels; ++index){
    //set noise
    double noise = 0.0 + 0.5 * index;
    std::cout << std::endl << std::endl << "***************************" << std::endl;
    std::cout << "Noise: " << noise << std::endl;
    Container aux_17points(noise, "17 points");
    Container aux_nonlinear(noise, "non lin");
    Container aux_ge(noise, "ge");
    Container aux_riemann(noise, "riemann grad");
    for(int index_stat = 0; index_stat < n_experiments; ++index_stat){
      //generate a random pose for viewpoint 1
      translation_t position1 = Eigen::Vector3d::Zero();
      rotation_t rotation1 = Eigen::Matrix3d::Identity();

      //generate a random pose for viewpoint 2
      translation_t position2 = generateRandomTranslation(2.0);
      rotation_t rotation2 = generateRandomRotation(0.5);

      //create a fake central camera
      translations_t camOffsets;
      rotations_t camRotations;
      generateRandomCameraSystem( numberCameras, camOffsets, camRotations );

      //derive correspondences based on random point-cloud
      bearingVectors_t bearingVectors1;
      bearingVectors_t bearingVectors2;
      std::vector<int> camCorrespondences1;
      std::vector<int> camCorrespondences2;
      Eigen::MatrixXd gt(3,numberPoints);
      generateRandom2D2DCorrespondences(
					position1, rotation1, position2, rotation2,
					camOffsets, camRotations, numberPoints, noise, outlierFraction,
					bearingVectors1, bearingVectors2,
					camCorrespondences1, camCorrespondences2, gt );

      //Extract the relative pose
      translation_t position; rotation_t rotation;
      extractRelativePose(
			  position1, position2, rotation1, rotation2, position, rotation, false );

      //print experiment characteristics
      printExperimentCharacteristics( position, rotation, noise, outlierFraction );

      //create non-central relative adapter
      relative_pose::NoncentralRelativeAdapter adapter(
						       bearingVectors1,
						       bearingVectors2,
						       camCorrespondences1,
						       camCorrespondences2,
						       camOffsets,
						       camRotations,
						       position,
						       rotation);

      //timer
      struct timeval tic;
      struct timeval toc;
      size_t iterations = 100;

      //running experiment
      std::cout << "running sixpt with 6 correspondences" << std::endl;
      std::vector<int> indices6 = getNindices(6);
      rotations_t sixpt_rotations;
      gettimeofday( &tic, 0 );
      for( size_t i = 0; i < iterations; i++ )
      {
	  sixpt_rotations = relative_pose::sixpt(adapter,indices6);
      }
      gettimeofday( &toc, 0 );
      double sixpt_time = TIMETODOUBLE(timeval_minus(toc,tic)) / iterations;
  
      std::cout << "running ge with 8 correspondences" << std::endl;
      std::vector<int> indices8 = getNindices(8);
      transformation_t ge_transformation;
      gettimeofday( &tic, 0 );
      /*for( size_t i = 0; i < iterations; i++ )
      {
	ge_transformation = relative_pose::ge(adapter,indices8);
      }*/
      ge_transformation = relative_pose::ge(adapter,indices8);
      gettimeofday( &toc, 0 );
      //double ge_time = TIMETODOUBLE(timeval_minus(toc,tic)) / iterations;
      double ge_time = TIMETODOUBLE(timeval_minus(toc,tic));
      std::cout << "running seventeenpt algorithm with 17 correspondences";
      std::cout << std::endl;
      std::vector<int> indices17 = getNindices(17);
      transformation_t seventeenpt_transformation;
      gettimeofday( &tic, 0 );
      /*for(size_t i = 0; i < iterations; i++)
      {
	  seventeenpt_transformation = relative_pose::seventeenpt(adapter,indices17);
      }*/
      seventeenpt_transformation = relative_pose::seventeenpt(adapter,indices17);
      gettimeofday( &toc, 0 );
      //double seventeenpt_time = TIMETODOUBLE(timeval_minus(toc,tic)) / iterations;
      double seventeenpt_time = TIMETODOUBLE(timeval_minus(toc,tic));
      std::cout << "running seventeenpt algorithm with all correspondences";
      std::cout << std::endl;
      transformation_t seventeenpt_transformation_all;
      gettimeofday( &tic, 0 );
      for(size_t i = 0; i < iterations; i++)
      {
	seventeenpt_transformation_all = relative_pose::seventeenpt(adapter);
      }
      gettimeofday( &toc, 0 );
      double seventeenpt_time_all =
	TIMETODOUBLE(timeval_minus(toc,tic)) / iterations;

      std::cout << "setting perturbed pose and ";
      std::cout << "performing nonlinear optimization" << std::endl;
      translation_t t_perturbed; rotation_t R_perturbed;
      getPerturbedPose( position, rotation, t_perturbed, R_perturbed, 0.1);
      transformation_t nonlinear_transformation;
      gettimeofday( &tic, 0 );
      /*for(size_t i = 0; i < iterations; i++)
      {
	adapter.sett12(t_perturbed);
	adapter.setR12(R_perturbed);
	nonlinear_transformation = relative_pose::optimize_nonlinear(adapter);
      }*/
      adapter.sett12(t_perturbed);
      adapter.setR12(R_perturbed);
      nonlinear_transformation = relative_pose::optimize_nonlinear(adapter);
      gettimeofday( &toc, 0 );
      //double nonlinear_time = TIMETODOUBLE(timeval_minus(toc,tic)) / iterations;
      double nonlinear_time = TIMETODOUBLE(timeval_minus(toc,tic));

      std::cout << "setting perturbed pose and ";
      std::cout << "performing nonlinear optimization with 10 correspondences";
      std::cout << std::endl;
      std::vector<int> indices10 = getNindices(10);
      getPerturbedPose( position, rotation, t_perturbed, R_perturbed, 0.1);
      adapter.sett12(t_perturbed);
      adapter.setR12(R_perturbed);
      transformation_t nonlinear_transformation_10 =
	relative_pose::optimize_nonlinear(adapter,indices10);
      double tol = 1e-12;
      Eigen::Matrix3d R = seventeenpt_transformation.block<3,3>(0,0);
      Eigen::Matrix3d error2 = Eigen::Matrix3d::Constant(3,3,0.0);
      error2(0,0) = 0.906806914003219 ; error2(0,1) = 0.376274665066534 ; error2(0,2) = -0.190048933554036;
      error2(1,0) = -0.357610129211461; error2(1,1) = 0.925360354683881 ; error2(1,2) = 0.125790339313809;
      error2(2,0) = 0.223195466354766 ; error2(2,1) = -0.046104125719814; error2(2,2) = 0.973682799165257;
      Eigen::Matrix3d R_ = R * error2;
      gettimeofday( &tic, 0 );
      transformation_t optimizer_algorithm;
      /*for(size_t i = 0; i < iterations; i++)
      {
	optimizer_algorithm = relative_pose::egea(adapter, tol, R_, numberPoints);
	}*/
      optimizer_algorithm = relative_pose::egea(adapter, tol, R_, numberPoints);
      gettimeofday( &toc, 0 );
      //double opt_time = TIMETODOUBLE(timeval_minus(toc,tic)) / iterations;
      double opt_time = TIMETODOUBLE(timeval_minus(toc,tic));
      aux_riemann.add_error_information(rotation, optimizer_algorithm.block<3,3>(0,0),
					position, optimizer_algorithm.block<3,1>(0,3), opt_time);
      //print results
      /*std::cout << "results from 6pt algorithm:" << std::endl;
      for( size_t i = 0; i < sixpt_rotations.size(); i++ )
      {
	std::cout << sixpt_rotations[i] << std::endl << std::endl;
	}*/
      std::cout << "result from ge using 8 points:" << std::endl;
      std::cout << ge_transformation << std::endl << std::endl;
      std::cout << "results from 17pt algorithm:" << std::endl;
      std::cout << seventeenpt_transformation << std::endl << std::endl;
      aux_17points.add_error_information(rotation, seventeenpt_transformation.block<3,3>(0,0),
					 position, seventeenpt_transformation.block<3,1>(0,3), seventeenpt_time);
      /*std::cout << "results from 17pt algorithm with all points:" << std::endl;
	std::cout << seventeenpt_transformation_all << std::endl << std::endl;*/
      std::cout << "results from nonlinear algorithm:" << std::endl;
      std::cout << nonlinear_transformation << std::endl << std::endl;
      aux_nonlinear.add_error_information(rotation, nonlinear_transformation.block<3,3>(0,0),
					  position, nonlinear_transformation.block<3,1>(0,3), nonlinear_time);
      /*std::cout << "results from nonlinear algorithm with only few correspondences:";
      std::cout << std::endl;
      std::cout << nonlinear_transformation_10 << std::endl << std::endl;*/

      std::cout << "results from the new algorithm with only few correspondences:";
      std::cout << std::endl;
      std::cout << optimizer_algorithm << std::endl << std::endl;

      /*std::cout << "timings from 6pt algorithm: ";
      std::cout << sixpt_time << std::endl;
      std::cout << "timings from ge: ";
      std::cout << ge_time << std::endl;*/
      aux_ge.add_error_information(rotation, ge_transformation.block<3,3>(0,0),
				   position, ge_transformation.block<3,1>(0,3), ge_time);
      /*std::cout << "timings from 17pt algorithm: ";
      std::cout << seventeenpt_time << std::endl;
      std::cout << "timings from 17pt algorithm with all the points: ";
      std::cout << seventeenpt_time_all << std::endl;
      std::cout << "timings from nonlinear algorithm: ";
      std::cout << nonlinear_time << std::endl;*/
    }
    information_list17pt.push_back(aux_17points);
    information_listNL.push_back(aux_nonlinear);
    information_ge.push_back(aux_ge);
    information_riemann.push_back(aux_riemann);
  }
  std::ofstream datafile;
  datafile.open("data.csv");
  for(unsigned int i = 0; i < information_list17pt.size(); ++i){
    information_list17pt[i].printInfo(datafile);
  }
  for(unsigned int i = 0; i < information_listNL.size(); ++i){
    information_listNL[i].printInfo(datafile);
  }
  for(unsigned int i = 0; i < information_ge.size(); ++i){
    information_ge[i].printInfo(datafile);
  }
  for(unsigned int i = 0; i < information_riemann.size(); ++i){
    information_riemann[i].printInfo(datafile);
  }
  datafile.close();
}
