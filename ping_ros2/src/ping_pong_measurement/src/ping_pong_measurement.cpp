/**
 * @file ping_pong_measurement.cpp
 * @brief Ping Pong Measurement Node
 * 
 * The following was referenced.
 * 
 * ROS 2 Tutorials: Writing a Simple Publisher and Subscriber (C++)
 * https://docs.ros.org/en/humble/Tutorials/Beginner-Client-Libraries/Writing-A-Simple-Cpp-Publisher-And-Subscriber.html
 * 
 * takasehideki, "mROS2 eval", GitHub, 2021
 * https://github.com/mROS-base/eval/blob/29c972a8c21bbaf81832d1f044b206c85edd31ae/host_ws/src/mros2_eval_uint16/src/pub_node.cpp
 * 
 */

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <fstream>
#include <sys/mman.h>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32_multi_array.hpp"

using namespace std::chrono_literals;
using std::placeholders::_1;

#define NUM_EVAL 2000
#define NUM_PUB 2050
#define EVAL_INTERVAL 50ms
#define ARRAY_SIZE 4

std::string prefix;

class PingPong : public rclcpp::Node
{
  public:
    PingPong() : Node("ping_pong"), pub_count_(0), sub_count_(0)
    {
      publisher_ = this->create_publisher<std_msgs::msg::Int32MultiArray>("ping", rclcpp::QoS(1));
      subscriber_ = this->create_subscription<std_msgs::msg::Int32MultiArray>("pong", rclcpp::QoS(1).best_effort(), std::bind(&PingPong::topic_callback, this, _1));
      rclcpp::sleep_for(1s); // wait for subscriber to be ready
      
      timer_ = this->create_wall_timer(EVAL_INTERVAL, std::bind(&PingPong::timer_callback, this));
    }

  private:
    std::array<int32_t, NUM_PUB> publogs;
    std::array<int32_t, NUM_EVAL> sublogs;
    std::array<rclcpp::Time, NUM_PUB> pubtimelogs;
    std::array<rclcpp::Time, NUM_EVAL> subtimelogs;

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<std_msgs::msg::Int32MultiArray>::SharedPtr publisher_;
    rclcpp::Subscription<std_msgs::msg::Int32MultiArray>::SharedPtr subscriber_;
    uint pub_count_, sub_count_;

    void timer_callback()
    {
      // publish
      if (pub_count_ < NUM_PUB && sub_count_ < NUM_EVAL)
      {
        // create message
        auto message = std_msgs::msg::Int32MultiArray();
        message.data.push_back(pub_count_);
        for(int i=1; i<ARRAY_SIZE; i++){
          message.data.push_back(rand()); // 0~32767
        }

        // store publish data in array
        publogs[pub_count_] = pub_count_;
        pubtimelogs[pub_count_] = this->get_clock()->now(); // get current time
        
        publisher_->publish(message);
        pub_count_++;
      }

      // record
      else{
        std::ofstream writing_file;
        std::string filename = prefix + "pub.csv";
        writing_file.open(filename, std::ios::out);
        if (!writing_file)
        {
          RCLCPP_ERROR(this->get_logger(), "writing file not found!!");
          rclcpp::shutdown();
        }

        for (uint i=0; i<pub_count_; i++)
        {
          const std::string writing_text = std::to_string(i) + "," 
            + std::to_string(pubtimelogs[i].nanoseconds()) + "," 
            + std::to_string(publogs[i]);
          writing_file << writing_text << std::endl;
        }

        RCLCPP_INFO(this->get_logger(), "eval on pub completed: %d", pub_count_);
        timer_->cancel(); // stop timer
        rclcpp::shutdown();
      }
    }

    // subscribe callback
    void topic_callback(const std_msgs::msg::Int32MultiArray::SharedPtr message)
    {
      if (sub_count_ < NUM_EVAL)
      {
        subtimelogs[sub_count_] = this->get_clock()->now();
        sublogs[sub_count_] = message->data[0];
        RCLCPP_INFO(this->get_logger(), "Subscribing: '%d'", sub_count_);
      }
      sub_count_++;

      if (sub_count_ == NUM_EVAL)
      {
        std::ofstream writing_file;
        std::string filename = prefix + "sub.csv";
        writing_file.open(filename, std::ios::out);
        if (!writing_file)
        {
          RCLCPP_ERROR(this->get_logger(), "writing file not found!!");
          rclcpp::shutdown();
        }
        for (int i=0; i<NUM_EVAL; i++)
        {
          const std::string writing_text = std::to_string(i) + "," 
            + std::to_string(subtimelogs[i].nanoseconds()) + ","
            + std::to_string(sublogs[i]);
          writing_file << writing_text << std::endl;
        }
        RCLCPP_INFO(this->get_logger(), "eval on sub completed");
        subscriber_.reset(); // stop subscriber 
      }
    }

};


int main(int argc, char * argv[])
{
  mlockall(MCL_FUTURE);
  sched_param pri = {94};
  sched_setscheduler(0, SCHED_FIFO, &pri);

  if (argc < 2)
  {
    prefix = "";
  }
  else
  {
    prefix = argv[1];
  }
  
  rclcpp::init(argc, argv);

  // rclcpp::executors::MultiThreadedExecutor executor;
  // auto ping_pong = std::make_shared<PingPong>();
  // executor.add_node(ping_pong);
  // executor.spin();

  rclcpp::spin(std::make_shared<PingPong>());

  rclcpp::shutdown();
  return 0;
}
