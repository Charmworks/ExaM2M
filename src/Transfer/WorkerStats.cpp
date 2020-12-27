// *****************************************************************************
/*!
  \file      src/Transfer/WorkerStats.cpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Chare class declaration for getting statistics from Worker
  \details   Chare class declaration for getting statistics from Worker
*/
// *****************************************************************************

#include "Worker.hpp"
#include <assert.h>

namespace exam2m {

  // Custom reduction function for getting global bbox min, max
  CkReductionMsg *getBboxDimMinMax(int nMsg, CkReductionMsg **msgs) {

    double *result = (double *)msgs[0]->getData();

    // Iterate over received msgs to determine min and max
    for(int i=1; i < nMsg; i++) {
      double *contrib = (double *)msgs[i]->getData();

      for(int j=0; j < 3; j++) {
        if(contrib[j] < result[j])
          result[j] = contrib[j]; // new min
      }
      for(int j=3; j < 6; j++) {
        if(contrib[j] > result[j])
          result[j] = contrib[j]; // new max
      }
    }
    // contribute computed result for reduction
    return CkReductionMsg::buildNew(6*sizeof(double), result);
  }

  // Custom reduction function for getting histogram of bbox dimensions
  CkReductionMsg *getBboxDimHist(int nMsg, CkReductionMsg **msgs) {

    int *result = (int *)msgs[0]->getData();
    int numBuckets = result[0]; // zeroth index reserved to store numBuckets

    int arrSize = 1 + 3 * numBuckets;

    for(int i=1; i < nMsg; i++) { // Sum contributions for all msgs
      int *contrib = (int *)msgs[i]->getData();
      for(int j=0; j < 3; j++) { // Iterate over x, y, z
        // Sum contribution for all numBuckets
        for(int k=0; k < numBuckets; k++) {
          result[1 + j*numBuckets + k] += contrib[1 + j*numBuckets + k];
        }
      }
    }
    return CkReductionMsg::buildNew(arrSize * sizeof(int), result);
  }
}


using exam2m::WorkerStats;

WorkerStats::WorkerStats(
  CProxy_Worker workerProxy) :
  m_workerProxy(workerProxy),
  numBuckets(10) {}

void WorkerStats::receiveMinMaxData(CkReductionMsg *msg) {

  double *result = (double *)(msg->getData());

  // compute bucketSize along each dim
  for(int j=0; j < 3; j++) {
    minData[j] = result[j];
    bucketSize[j] = (result[j + 3] - result[j])/(numBuckets);
  }
  // bcast the bucketSize and numBuckets to all workers
  m_workerProxy.computeHist(bucketSize, result, result + 3, numBuckets);
}


void WorkerStats::receiveHistData(CkReductionMsg *msg) {

  int *result = (int *)(msg->getData());
  std::string axis = "";

  CkPrintf("ExaM2M> Histogram computed for Tet bboxes, numBuckets=%d\n", numBuckets);
  for(int j=0; j < 3; j++) {
    CkPrintf("=============================================================\n");
    if(j == 0) axis = "X";
    else if(j == 1) axis = "Y";
    else if(j == 2) axis = "Z";

    for(int i=0; i<numBuckets; i++) {
      double low = minData[j] + i * bucketSize[j];
      double high = minData[j] + (i + 1) * bucketSize[j];
      CkPrintf("Axis=%s, Interval = [%lf, %lf], Num Entries=%d\n", axis.c_str(), low, high, result[ 1 + j * numBuckets + i]);
    }
  }
  CkPrintf("=============================================================\n");
}
