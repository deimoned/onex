#ifndef GROUPABLE_TIME_SERIES_SET_H
#define GROUPABLE_TIME_SERIES_SET_H

#include "TimeSeriesSet.hpp"
#include "GlobalGroupSpace.hpp"
#include <vector>

#include "distance/Distance.hpp"

#define GROUP_FILE_VERSION 1

namespace onex {

/**
 *  @brief a GroupableTimeSeriesSet object is a TimeSeriesSet with grouping
 *         functionalities
 */
class GroupableTimeSeriesSet : public TimeSeriesSet
{
public:

  /**
   *  @brief Destructor
   */
  ~GroupableTimeSeriesSet();

  /**
   *  @brief groups the datset into similarity groups
   *
   *  @param distance_name the distance to use for comparing similarity
   *  @param threshold to use for determing the bound of similarity
   *
   *  @return the number of groups created
   */
  int groupAllLengths(const std::string& distance_name, data_t threshold);

  /**
    *  @brief deletes and clears the groups
    */
  void reset();

  /**
    *  @brief check if the dataset is grouped
    */
  bool isGrouped() const;

  void saveGroups(const std::string& path, bool groupSizeOnly) const;
  int loadGroups(const std::string& path);
  
  /**
   * @brief Finds the best matching subsequence in the dataset
   *
   * @param other the timeseries to find the match for
   *
   * @return a struct containing the closest TimeSeries and the distance between them
   * @throws exception if dataset is not grouped
   */
  candidate_time_series_t getBestMatch(const TimeSeries& other) const;

private:
  GlobalGroupSpace* groupsAllLengthSet = nullptr;
  data_t threshold;
};

} // namespace onex

#endif // GROUPABLE_TIME_SERIES_SET_H