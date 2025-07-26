#ifndef PHYSICAL_CUBE_H
#define PHYSICAL_CUBE_H

#include "VirtualCube.h"

constexpr int MOTOR_DRIVER_IN1[6] = { 3, 5, 6, 9, 10, 11 };  // IN_1 핀들은 PWM 핀. 즉, 아날로그 출력을 담당합니다
constexpr int MOTOR_DRIVER_IN2[6] = { 2, 4, 7, 8, 12, 13 };  // IN_2 핀들은 일반 디지털 핀으로 디지털 출력을 담당합니다.
constexpr int SENSOR_OUT[6] = { A0, A1, A2, A3, A4, A5 };    // 디지털 핀을 모두 사용해서 아날로그 핀이 디지털 입력을 담당합니다.
// 아래 두 배열에 센서로 측정한 모터의 회전 수치를 저장합니다.
int axis_old_rotation[6] = {};  // 축을 인덱스로 직전 프레임의 각 면의 회전 수치를 저장합니다.
int axis_now_rotation[6] = {};  // 축을 인덱스로 현재 프레임의 각 면의 회전 수치를 저장합니다.
// 마지막으로 회전을 감지한 시간을 저장합니다.
// 이를 계산하기 위해 axis_old_rotation를 정의하고 사용합니다.
uint32_t last_rotated;

/* 1. 센서와 모터
 * 
 * sensor_read()의 출력값은 모터의 순간의 회전값이 아닌, 현재의 정확한 회전값을 반환한다는 것을 전제로 만들었습니다.
 * 결과값은 [0,1024)라고 가정했습니다.
 * 
 * motor_rotate_cw()와 motor_rotate_ccw()는 큐브의 면을 일정한 속도로 회전시킵니다.
 * 방향은 각각 시계, 반시계 방향입니다.
 * 또한 아래 주소에서 제공하는 예제 코드를 가져온 겁이므로 수정의 여지가 있으며,
 *  코드의 최적화 역시 직접 모터 드라이버를 사용하고 이뤄질 듯 합니다.
 * >> https://wiki.dfrobot.com/Dual_1.5A_Motor_Driver_-_HR8833_SKU__DRI0040#Sample%20code
 * 
 * motor_write()는 두 함수, motor_rotate_cw()와 motor_rotate_ccw()를 이용해서 모터를 어떠한 각도까지 회전시킵니다.
 */
int sensor_read(int axis) {  // 센서의 값을 읽어오는 함수입니다.
  return digitalRead(SENSOR_OUT[axis]);
}
void motor_rotate_cw(int axis, int speed) {  // 모터를 시계방향으로 회전시키는 함수입니다.
  analogWrite(MOTOR_DRIVER_IN1[axis], speed);
  digitalWrite(MOTOR_DRIVER_IN2[axis], LOW);
}
void motor_rotate_ccw(int axis, int speed) {  // 모터를 반시계방향으로 회전시키는 함수입니다.
  int speed2 = 255 - speed;
  analogWrite(MOTOR_DRIVER_IN1[axis], speed2);
  digitalWrite(MOTOR_DRIVER_IN2[axis], HIGH);
}
void motor_write(int axis, int target_rotate) {  // 목표 회전값까지 회전하도록 하는 함수입니다.
  for (int delta_rotate = (target_rotate - sensor_read(axis) + 1024) % 1024;
       !(-3 <= delta_rotate && delta_rotate <= 3);
       delta_rotate = (target_rotate - sensor_read(axis) + 1024) % 1024) {
    if (/*0 <= delta_rotate &&*/ delta_rotate < 512) {
      motor_rotate_cw(axis, 50);
    } else {
      motor_rotate_ccw(axis, 50);
    }
  }
}

// 큐브의 회전값을 받아서 그 방향을 0, 1, 2, 3의 숫자 중 하나로 반환합니다.
// 큐브는 물리적으로 하나의 면이 완전히 맞닿은 직각으로 회전해야 다른 면이 회전할 수 있으므로 다른 면의 회전은 확인하지 않습니다.
int get_cube_area(int rotation) {
  return (rotation + 128) / 256 /*% 4*/;  // 원래라면 "% 4"를 붙이는 게 맞지만, 그 사용 과정에 "% 4"가 포함되어 있기 때문에 생략합니다.
}

// 변수들을 초기화하는 함수입니다.
void cube_init() {
  for (int axis = 0; axis < 6; ++axis) {
    // 핀 입출력 설정입니다.
    pinMode(MOTOR_DRIVER_IN1[axis], OUTPUT);
    pinMode(MOTOR_DRIVER_IN2[axis], OUTPUT);
    pinMode(SENSOR_OUT[axis], INPUT);

    // 모든 축의 회전(HW)을 0으로 초기화하고, 회전값을 저장하는 변수(SW)들도 모두 0(또는 입력값)으로 초기화합니다.
    motor_write(axis, 0);
    axis_old_rotation[axis] = axis_now_rotation[axis] = sensor_read(axis);
  }

  // init() 종료 시간을 프로그램 시작 시간으로 생각합니다.
  last_rotated = millis();
}

/* 2. 메인 로직 함수
 * 
 * 모터의 입력을 처리하는 부분입니다.
 * 즉 전체 로직의 주요 루프가 되는 함수로, 메인 로직이라고도 볼 수 있습니다.
 */
void cube_update() {
  // 모든 축의 회전을 감지해서 그 수치가 일정치를 넘으면 VirtualCube의 회전을 수행합니다.
  for (int axis = 0; axis < 6; ++axis) {
    axis_old_rotation[axis] = axis_now_rotation[axis];
    axis_now_rotation[axis] = sensor_read(axis);
    if (axis_old_rotation[axis] != axis_now_rotation[axis]) {  // 회전이 감지되지 않았을 경우입니다.
      int old_area = get_cube_area(axis_old_rotation[axis]);
      int now_area = get_cube_area(axis_now_rotation[axis]);

      rotate(static_cast<Color>(axis), (now_area - old_area + 4) % 4);
      last_rotated = millis();
    }
  }

  // 마지막으로 회전값이 변한 이후로 3초 이상 지나면 해법을 계산 및 수행하도록 합니다.
  if (millis() - last_rotated >= 3000) {
    // 해법을 계산 및 수행합니다.
    solve();
    for (char c : container) {
      switch (c) {
        case 'W': motor_write(0, (sensor_read(0) + 256) % 1024); break; // 90도 우회전
        case 'Y': motor_write(1, (sensor_read(1) + 256) % 1024); break;
        case 'G': motor_write(2, (sensor_read(2) + 256) % 1024); break;
        case 'B': motor_write(3, (sensor_read(3) + 256) % 1024); break;
        case 'O': motor_write(4, (sensor_read(4) + 256) % 1024); break;
        case 'R': motor_write(5, (sensor_read(5) + 256) % 1024); break;

        case 'w': motor_write(0, (sensor_read(0) + 768) % 1024); break; // 90도 좌회전
        case 'y': motor_write(1, (sensor_read(1) + 768) % 1024); break;
        case 'g': motor_write(2, (sensor_read(2) + 768) % 1024); break;
        case 'b': motor_write(3, (sensor_read(3) + 768) % 1024); break;
        case 'o': motor_write(4, (sensor_read(4) + 768) % 1024); break;
        case 'r': motor_write(5, (sensor_read(5) + 768) % 1024); break;
      }
    }

    // 만일을 대비한 delay() 입니다
    delay(500);
  }
}

#endif  // !PHYSICAL_CUBE_H