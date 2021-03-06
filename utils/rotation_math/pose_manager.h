#pragma once

#include <assert.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <utility>
#include <vector>

#include "utils/cuda/lie_group.cuh"

using timed_pose_tuple = std::pair<int64_t, SE3<float>>;

class pose_manager {
 public:
  pose_manager();

  void register_valid_pose(const int64_t timestamp, const SE3<float>& pose);

  void register_valid_pose(const int64_t timestamp, const Eigen::Matrix4d& pose);

  SE3<float> query_pose(const int64_t timestamp);

  SE3<float> get_latest_pose();

 private:
  // get index to the maximum timestamp that is smaller than the given
  // timestamp.
  uint64_t get_max_lower_idx(const int64_t timestamp, uint64_t start_idx, uint64_t end_idx);

  std::vector<timed_pose_tuple> timed_pose_vec;

  std::mutex vec_lock;
};