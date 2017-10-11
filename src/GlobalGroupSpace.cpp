#include "GlobalGroupSpace.hpp"
#include "LocalLengthGroupSpace.hpp"
#include <cmath>
#include <sstream>
#include <functional>
#include <queue>
#include <vector>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "TimeSeries.hpp"
#include "TimeSeriesSet.hpp"
#include "distance/Distance.hpp"
#include "Group.hpp"

using std::vector;
using std::max;
using std::min;
using std::cout;
using std::endl;
using std::ofstream;
using std::ifstream;
using std::string;

namespace onex {

void GlobalGroupSpace::reset(void)
{
  for (unsigned int i = 0; i < this->localLengthGroupSpace.size(); i++) {
    delete this->localLengthGroupSpace[i];
    this->localLengthGroupSpace[i] = nullptr;
  }
  this->localLengthGroupSpace.clear();
}

void GlobalGroupSpace::loadDistance(const string& distance_name)
{
  this->distanceName = distance_name;
  this->pairwiseDistance = getDistance(distance_name);
  this->warpedDistance = cascadeDistance;
}

int GlobalGroupSpace::group(const string& distance_name, data_t threshold)
{
  reset();
  this->loadDistance(distance_name);
  this->localLengthGroupSpace.resize(dataset.getItemLength() + 1, nullptr);
  this->threshold = threshold;
  int numberOfGroups = 0;

  for (unsigned int i = 2; i < this->localLengthGroupSpace.size(); i++)
  {
    this->localLengthGroupSpace[i] = new LocalLengthGroupSpace(dataset, i);
    int noOfGenerated = this->localLengthGroupSpace[i]->generateGroups(this->pairwiseDistance, threshold);
    numberOfGroups += noOfGenerated;
  }
  return numberOfGroups;
}

candidate_time_series_t GlobalGroupSpace::getBestMatch(const TimeSeries& query)
{
  if (query.getLength() <= 1) {
    throw OnexException("Length of query must be larger than 1");
  }
  data_t bestSoFarDist = INF;
  const Group* bestSoFarGroup = nullptr;

  vector<int> order (generateTraverseOrder(query.getLength(), this->localLengthGroupSpace.size() - 1));
  for (unsigned int io = 0; io < order.size(); io++) {
    int i = order[io];
    // this looks through each group of a certain length finding the best of those groups
    candidate_group_t candidate = this->localLengthGroupSpace[i]->getBestGroup(query, this->warpedDistance, bestSoFarDist);
    if (candidate.second < bestSoFarDist)
    {
      bestSoFarGroup = candidate.first;
      bestSoFarDist = candidate.second;
    }
  }
  return bestSoFarGroup->getBestMatch(query, this->warpedDistance);
}

bool GlobalGroupSpace::grouped(void) const
{
  return localLengthGroupSpace.size() > 0;
}

void GlobalGroupSpace::saveGroups(ofstream &fout, bool groupSizeOnly) const
{
  // Range of lengths and distance name
  fout << 2 << " " << this->localLengthGroupSpace.size() << endl;
  fout << this->distanceName << endl;
  for (unsigned int i = 2; i < this->localLengthGroupSpace.size(); i++) {
    this->localLengthGroupSpace[i]->saveGroups(fout, groupSizeOnly);
  }
}

int GlobalGroupSpace::loadGroups(ifstream &fin)
{
  reset();

  int lenFrom, lenTo;
  int numberOfGroups = 0;
  string distance;
  fin >> lenFrom >> lenTo >> distance;
  boost::trim_right(distance);
  this->loadDistance(distance);
  this->localLengthGroupSpace.resize(dataset.getItemLength() + 1, nullptr);  
  for (unsigned int i = lenFrom; i < lenTo; i++) {
    LocalLengthGroupSpace* gel = new LocalLengthGroupSpace(dataset, i);
    numberOfGroups += gel->loadGroups(fin);
    this->localLengthGroupSpace[i] = gel;
  }
  return numberOfGroups;
}

vector<int> generateTraverseOrder(int queryLength, int totalLength)
{
  vector<int> order;
  int low = queryLength - 1;
  int high = queryLength + 1;
  bool lowStop = false, highStop = false;

  order.push_back(queryLength);
  while (!(lowStop && highStop)) {
    if (low < 2) lowStop = true;
    if (high > totalLength) highStop = true;

    if (!lowStop) {
      // queryLength is always larger than low
      int r = calculateWarpingBandSize(queryLength);
      if (low + r >= queryLength) {
        order.push_back(low);
        low--;
      }
      else {
        lowStop = true;
      }
    }

    if (!highStop) {
      // queryLength is always smaller than high
      int r = calculateWarpingBandSize(high);
      if (queryLength + r >= high) {
        order.push_back(high);
        high++;
      }
      else {
        highStop = true;
      }
    }
  }
  return order;
}

} // namespace onex
