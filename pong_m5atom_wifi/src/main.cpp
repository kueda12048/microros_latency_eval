#include <M5Atom.h>
#include <micro_ros_arduino.h>

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32_multi_array.h>

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}} /// エラーチェック
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}

rcl_publisher_t publisher;
rcl_subscription_t subscriber;
std_msgs__msg__Int32MultiArray msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

void error_loop(){
  while(1){
    M5.dis.drawpix(0, 0x000f00);
    delay(500);
    M5.dis.drawpix(0, 0x000000);
    delay(500);
  }
}

void led_blink()
{
  static int FSM = 0;
  M5.dis.drawpix(FSM/4, 0x0000f0);
  M5.dis.drawpix((FSM/4+25-1)%25, 0x000000);
  FSM++;
    if (FSM >= 25*4)
    {
        FSM = 0;
    }
  M5.update();
}

void subscription_callback(const void * msgin)
{
  RCSOFTCHECK(rcl_publish(&publisher, msgin, NULL));
  led_blink();
}



void setup() {
  // put your setup code here, to run once:
  M5.begin(true, false, true);
  M5.dis.drawpix(0, 0x0000f0);

  /// wifi経由
  set_microros_wifi_transports(__SSID__, __SSID_PASS__, "192.168.11.20", 8888);
  delay(2000);

  allocator = rcl_get_default_allocator();
  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
  // create nodes
  RCCHECK(rclc_node_init_default(&node, "micro_ros_arduino_node", "", &support));

  // create publisher
  RCCHECK(rclc_publisher_init_default(
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
