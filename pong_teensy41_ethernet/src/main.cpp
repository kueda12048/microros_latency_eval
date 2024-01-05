/**
 * @file main.cpp
 * @brief 
 * https://github.com/micro-ROS/micro_ros_arduino/blob/galactic/examples/micro-ros_publisher_ethernet/micro-ros_publisher_ethernet.ino
 */
#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32_multi_array.h>

#if !defined(TARGET_STM32F4) && !defined(ARDUINO_TEENSY41) && !defined(TARGET_PORTENTA_H7_M7)
#error This example is only available for Arduino Portenta, Arduino Teensy41 and STM32F4
#endif

#if defined(ARDUINO_TEENSY41)
void get_teensy_mac(uint8_t *mac) {
    for(uint8_t by=0; by<2; by++) mac[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
    for(uint8_t by=0; by<4; by++) mac[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;
}
#endif

rcl_publisher_t publisher;
rcl_subscription_t subscriber;
std_msgs__msg__Int32MultiArray msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

#define LED_PIN 13

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

void error_loop(){
  while(1){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
}

void subscription_callback(const void * msgin)
{
  RCSOFTCHECK(rcl_publish(&publisher, msgin, NULL));
}

void setup() {
   byte arduino_mac[] = { 0xAA, 0xBB, 0xCC, 0xEE, 0xDD, 0xFF };

  #if defined(ARDUINO_TEENSY41)
  get_teensy_mac(arduino_mac);
  #endif

  IPAddress arduino_ip(192, 168, 11, 177);
  IPAddress agent_ip(192, 168, 11, 20);
  set_microros_native_ethernet_udp_transports(arduino_mac, arduino_ip, agent_ip, 8888);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  delay(2000);

  allocator = rcl_get_default_allocator();

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "micro_ros_arduino_ethernet_node", "", &support));

  // create publisher
  RCCHECK(rclc_publisher_init_best_effort(
    &publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray),
    "pong"));

  // create subscriber
  RCCHECK(rclc_subscription_init_best_effort(
    &subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray),
    "ping"));

  msg.data.capacity = 100; 
  msg.data.size = 0;
  msg.data.data = (int32_t*) malloc(msg.data.capacity * sizeof(int32_t));

  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));
}

void loop() {
    RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10)));
}
