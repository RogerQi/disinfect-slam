#pragma once

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

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

  /**
   * L515 Camera Specs mandates a set of parameters to set up the
   * camera stream. Changing these parameters without following spec
   * may result in "Couldn't resolve requests" error.
   *
   * Spec: https://docs.rs-online.com/f31c/A700000006942953.pdf
   */
  static const int WIDTH = 1280;
  static const int HEIGHT = 720;
  static const int DEPTH_WIDTH = 640;
  static const int DEPTH_HEIGHT = 480;
  static const int FPS = 30;

 private:
  rs2::config cfg_;
  rs2::pipeline pipe_;
  rs2::pipeline_profile pipe_profile_;
  rs2::align align_to_color_;
};
