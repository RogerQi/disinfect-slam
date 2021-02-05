#pragma once

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>

#include <Eigen/Dense>
#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

struct imu_frame_t {
  Eigen::Vector3d gyro;
  Eigen::Vector3d accel;
  double timestamp;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
};

typedef std::vector<imu_frame_t, Eigen::aligned_allocator<imu_frame_t>> imu_sequence_t;

struct sensor_frame_t {
  rs2::frameset image_frames;
  imu_sequence_t imu_frames;
};

struct l515_data_t {
  cv::Mat rgb_img;
  cv::Mat depth_img;
  imu_sequence_t imu_frames;
  double timestamp;
};

// synchronize accel and gyro data with linear interpolation
class ImuSyncer {
 public:
  using CallbackFunc = std::function<void(const imu_frame_t& imu_frame)>;

  ImuSyncer(const CallbackFunc& callback);

  void AddMeasurement(const rs2::frame& frame) const;

 private:

  void TryInvokeSync() const;

  CallbackFunc callback_;
  mutable std::queue<rs2::motion_frame> accel_q_;
  mutable std::deque<rs2::motion_frame> gyro_q_;
};

/**
 * @brief L515 camer interface with librealsense2
 */
class L515 {
 public:
  L515();
  ~L515();

  /**
   * @return depth map multiplier
   */
  double DepthScale() const;

  /**
   * @brief read an RGBD frame
   *
   * @param color_img rgb image
   * @param depth_img depth image
   *
   * @return timestamp in system clock
   */
  int64_t GetRGBDFrame(cv::Mat* color_img, cv::Mat* depth_img) const;

  /**
   * @brief set capture properties through librealsense
   *
   * @param option  capture option
   * @param value   value to be set
   */
  void SetDepthSensorOption(const rs2_option option, const float value);

 private:
  rs2::config cfg_;
  rs2::pipeline pipe_;
  rs2::pipeline_profile pipe_profile_;
  rs2::align align_to_color_;
};

class L515Async {
 public:
  explicit L515Async(bool start = true);

  ~L515Async();

  void Start();

  void Stop();

  l515_data_t GetSyncedData() const;

  template <rs2_stream StreamType>
  rs2_intrinsics GetVideoIntrinsics() const {
    const auto stream = pipe_profile_.get_stream(StreamType);
    return stream.as<rs2::video_stream_profile>().get_intrinsics();
  }

  template <rs2_stream StreamType>
  rs2_motion_device_intrinsic GetMotionIntrincis() const {
    const auto stream = pipe_profile_.get_stream(StreamType);
    return stream.as<rs2::motion_stream_profile>().get_motion_intrinsics();
  }

  template <rs2_stream From, rs2_stream To>
  rs2_extrinsics GetExtrinsics() const {
    const auto from_stream = pipe_profile_.get_stream(From);
    const auto to_stream = pipe_profile_.get_stream(To);
    return from_stream.get_extrinsics_to(to_stream);
  }

 private:
  void HandleImu(const imu_frame_t& imu_frame) const;
  void HandleRawStream(const rs2::frame& frame) const;
  void TryInvokeSwap() const;

  double depth_scale_;
  bool started_;
  rs2::pipeline pipe_;
  rs2::pipeline_profile pipe_profile_;
  rs2::align align_to_color_;
  ImuSyncer imu_syncer_;
  mutable sensor_frame_t frame_read_;
  mutable sensor_frame_t frame_write_;
  mutable std::mutex mtx_read_;
  mutable std::condition_variable cv_read_;
  mutable std::mutex mtx_start_;
};
