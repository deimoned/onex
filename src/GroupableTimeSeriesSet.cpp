#include "GroupableTimeSeriesSet.hpp"
#include "GlobalGroupSpace.hpp"
#include "Exception.hpp"
#include "distance/Distance.hpp"
#include <iostream>

#include <fstream>

using std::ofstream;
using std::ifstream;
using std::cout;
using std::endl;

namespace onex {

GroupableTimeSeriesSet::~GroupableTimeSeriesSet()
{
  this->reset();
}

int GroupableTimeSeriesSet::groupAllLengths(const std::string& distance_name, data_t threshold)
{
  if (!this->isLoaded())
  {
    throw OnexException("No data to group");
  }

  // clear old groups
  reset();

  this->groupsAllLengthSet = new GlobalGroupSpace(*this);
  int cntGroups = this->groupsAllLengthSet->group(distance_name, threshold);
  this->threshold = threshold;
  return cntGroups;
}

bool GroupableTimeSeriesSet::isGrouped() const
{
  return this->groupsAllLengthSet != nullptr;
}

void GroupableTimeSeriesSet::reset()
{
  delete this->groupsAllLengthSet;
  this->groupsAllLengthSet = nullptr;
}

void GroupableTimeSeriesSet::saveGroups(const string& path, bool groupSizeOnly) const
{
  if (!this->isGrouped()) {
    throw OnexException("No group found");
  }

  ofstream fout(path);
  if (fout)
  {
    // Version of the file format, the threshold and the required dataset dimensions
    fout << GROUP_FILE_VERSION << " " 
         << this->threshold << " "
         << this->getItemCount() << " "
         << this->getItemLength() << endl;
    this->groupsAllLengthSet->saveGroups(fout, groupSizeOnly);
  }
  else
  {
    throw OnexException("Cannot open file");
  }
}

int GroupableTimeSeriesSet::loadGroups(const string& path)
{
  int numberOfGroups = 0;
  ifstream fin(path);
  if (fin)
  {
    int version, grpItemCount, grpItemLength;
    data_t threshold;
    fin >> version >> threshold >> grpItemCount >> grpItemLength;
    if (version != GROUP_FILE_VERSION)
    {
      throw OnexException("Incompatible file version");
    }
    if (grpItemCount != this->getItemCount())
    {
      throw OnexException("Incompatible item count");
    }
    if (grpItemLength != this->getItemLength())
    {
      throw OnexException("Incompatible item length");
    }
    cout << "Saved groups are compatible with the dataset" << endl;
    reset();
    this->threshold = threshold;
    this->groupsAllLengthSet = new GlobalGroupSpace(*this);
    numberOfGroups = this->groupsAllLengthSet->loadGroups(fin);
  }
  else
  {
    throw OnexException("Cannot open file");
  }
  return numberOfGroups;
}

candidate_time_series_t GroupableTimeSeriesSet::getBestMatch(const TimeSeries& query) const
{
  if (this->groupsAllLengthSet) //not nullptr
  {
    return this->groupsAllLengthSet->getBestMatch(query);
  }
  throw OnexException("Dataset is not grouped");
}

} // namespace onex