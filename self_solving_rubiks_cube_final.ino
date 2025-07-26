/* 0. 목차
 * 
 * VirtualCube   1: 1. 함수 인자로 사용하기 위한 enum
 * VirtualCube  29: 2. 큐브의 저장 방식
 * VirtualCube 133: 3. 그 긴거
 * 
 * PhysicalCube 16: 1. 센서와 모터
 * PhysicalCube 76: 2. 메인 로직 함수
 */

#include "VirtualCube.h"
#include "PhysicalCube.h"

void setup() {
  reset_cube();
  cube_init();
}

void loop() {
  cube_update();
}
