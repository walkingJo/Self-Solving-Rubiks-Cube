#ifndef VIRTUAL_CUBE_H
#define VIRTUAL_CUBE_H

/* 1. 함수 인자로 사용하기 위한 enum
 *    큐브의 방향 등의 의미도 내포하고 있습니다.
 *
 * 큐브의 각 면의 색을 변수처럼 사용하기 위한 enum입니다.
 * 각각의 방향, 색상, 인덱스는 아래와 같이 대응됩니다.
 * 
 * 방향: Up     Down   Front  Back   Left   Right
 * 색상: White  Yellow Green  Blue   Orange Red
 * 숫자: 0      1      2      3      4      5
 */
enum class Color : uint8_t {
  W = 0,  // White
  Y = 1,  // Yellow
  G = 2,  // Green
  B = 3,  // Blue
  O = 4,  // Orange
  R = 5,  // Red
};
char color_char[6] = {
  'W',
  'Y',
  'G',
  'B',
  'O',
  'R',
};
String container;

/* 2. 큐브의 저장 방식
 * 
 * 외부 대신 내부에 따로 저장한다는 내용입니다.
 * 큐브의 한 면에서 봤을 때, 코너와 엣지는 아래와 같습니다.
 *   코너 엣지 코너
 *   엣지 센터 엣지
 *   코너 엣지 코너
 */
uint8_t corner_pos[8];
uint8_t corner_ori[8];
uint8_t edge_pos[12];
uint8_t edge_ori[12];
// 큐브의 상태를 초기화하는 함수입니다.
inline void reset_cube() {
  for (int i = 0; i < 8; ++i) {
    corner_pos[i] = i;
    corner_ori[i] = 0;
  }
  for (int i = 0; i < 12; ++i) {
    edge_pos[i] = i;
    edge_ori[i] = 0;
  }
}
// 큐브의 축과 회전 방향을 인자로 큐브의 회전을 수행하는 함수입니다.
// count가 양수면 시계방향, 음수면 반시계방향 회전합니다.
inline void rotate(Color axis, uint8_t count) {
  constexpr uint8_t corner_rotation_target[6][4] = {
    // 축을 인덱스로 회전의 대상이 될 코너 큐브의 인덱스 목록을 사용합니다.
    { 1, 2, 3, 0 },  // U
    { 7, 6, 5, 4 },  // D
    { 3, 7, 4, 0 },  // F
    { 5, 6, 2, 1 },  // B
    { 6, 7, 3, 2 },  // L
    { 4, 5, 1, 0 },  // R
  };
  constexpr uint8_t edge_rotation_target[6][4] = {
    // 축을 인덱스로 회전의 대상이 될 엣지 큐브의 인덱스 목록을 사용합니다.
    { 1, 2, 3, 0 },    // U
    { 11, 10, 9, 8 },  // D
    { 7, 8, 4, 0 },    // F
    { 5, 10, 6, 2 },   // B
    { 6, 11, 7, 3 },   // L
    { 4, 9, 5, 1 },    // R
  };
  constexpr uint8_t corner_ori_delta[6][4] = {
    // 축을 인덱스로 (회전의 대상이 될 코너 큐브)의 추가 회전수 목록을 사용합니다.
    { 0, 0, 0, 0 },  // U
    { 0, 0, 0, 0 },  // D
    { 2, 1, 2, 1 },  // F
    { 1, 2, 1, 2 },  // B
    { 1, 2, 1, 2 },  // L
    { 1, 2, 1, 2 },  // R
  };
  constexpr uint8_t edge_ori_flip[6] = {
    // 축을 인덱스로 (회전의 대상이 될 엣지 큐브)의 추가 회전 여부 목록을 사용합니다.
    false,  // U
    false,  // D
    false,  // F
    false,  // B
    true,   // L
    true,   // R
  };

  count = (4 + count % 4) % 4;         // 회전 4번마다 순환하니까 count를 작은 양수로 변환합니다.
  int _axis = static_cast<int>(axis);  // axis을 int로 사용하기 위함입니다.

  for (int i = 0; i < count; ++i) {
    // container에 회전을 알파벳으로 기록합니다.
    // solve()에서는 해법을 찾기 전 container를 초기화하여 오염을 막습니다.
    container += color_char[_axis];
    // 코너의 회전입니다.
    uint8_t corner_pos_temp = corner_pos[corner_rotation_target[_axis][0]];
    uint8_t corner_ori_temp = corner_ori[corner_rotation_target[_axis][0]];
    for (int j = 0; j < 3; ++j) {
      corner_pos[corner_rotation_target[_axis][j]] = corner_pos[corner_rotation_target[_axis][j + 1]];
      corner_ori[corner_rotation_target[_axis][j]] = (corner_ori[corner_rotation_target[_axis][j + 1]] + corner_ori_delta[_axis][j]) % 3;
    }
    corner_pos[corner_rotation_target[_axis][3]] = corner_pos_temp;
    corner_ori[corner_rotation_target[_axis][3]] = (corner_ori_temp + corner_ori_delta[_axis][3]) % 3;
    // 엣지의 회전입니다.
    uint8_t edge_pos_temp = edge_pos[edge_rotation_target[_axis][0]];
    uint8_t edge_ori_temp = edge_ori[edge_rotation_target[_axis][0]];
    for (int j = 0; j < 3; ++j) {
      edge_pos[edge_rotation_target[_axis][j]] = edge_pos[edge_rotation_target[_axis][j + 1]];
      edge_ori[edge_rotation_target[_axis][j]] = edge_ori[edge_rotation_target[_axis][j + 1]] ^ edge_ori_flip[_axis];
    }
    edge_pos[edge_rotation_target[_axis][3]] = edge_pos_temp;
    edge_ori[edge_rotation_target[_axis][3]] = edge_ori_temp ^ edge_ori_flip[_axis];
  }
}
// 오른손 트위스트를 수행하는 함수입니다.
inline void twist_rhand(Color top, Color right) {
  rotate(right, 1);
  rotate(top, 1);
  rotate(right, -1);
  rotate(top, -1);
}
// 왼손 트위스트를 수행하는 함수입니다.
inline void twist_lhand(Color top, Color left) {
  rotate(left, -1);
  rotate(top, -1);
  rotate(left, 1);
  rotate(top, 1);
}
/* 3. 그 긴거 (큐브를 해결하는 방법을 구하는 함수)
 *    아무래도 505줄짜리 함수를 PPT에 담는 건 어렵지 않나 싶습니다.   
 * 
 * 해법은 container에 저장됩니다.
 * 알파벳은 각각의 축을 의미하며 대문자는 시계방향, 소문자는 반시계방향입니다.
 * 
 * 원리는 아래 링크를 참고 바랍니다.
 * >> https://www.youtube.com/watch?v=HgEySd_N-6w
 */
void solve() {
  container = "";
  constexpr Color colors_tmp[4] = {
    Color::G,
    Color::R,
    Color::B,
    Color::O,
  };

  // 1. 흰 십자가 맞추기
  for (bool white_edge_to_move_exist = false;; white_edge_to_move_exist = false) {
    for (uint8_t pos = 0; pos < 12; ++pos) {
      uint8_t nxt_pos = edge_pos[pos];
      if (nxt_pos != 0 && nxt_pos != 1 && nxt_pos != 2 && nxt_pos != 3) {
        continue;
      }

      if (edge_ori[pos] == 0 && nxt_pos == pos) {
        continue;
      }

      white_edge_to_move_exist = true;

      switch (pos) {
        case 0:
        case 1:
        case 2:
        case 3:
          {
            if (edge_ori[pos] == 0) {
              rotate(colors_tmp[pos], +1);
              rotate(Color::W, (4 + nxt_pos - pos) % 4 * +1);
              rotate(colors_tmp[pos], -1);
              rotate(Color::W, (4 + nxt_pos - pos) % 4 * -1);
            } else {
              rotate(colors_tmp[pos], +1);
              rotate(Color::W, (3 + nxt_pos - pos) % 4 * +1);
              rotate(colors_tmp[(pos + 1) % 4], +1);
              rotate(Color::W, (3 + nxt_pos - pos) % 4 * -1);
            }
            break;
          }
        case 4:
          {
            if (edge_ori[pos] == 0) {
              rotate(Color::W, (4 + nxt_pos - 0) % 4 * +1);
              rotate(Color::G, -1);
              rotate(Color::W, (4 + nxt_pos - 0) % 4 * -1);
            } else {
              rotate(Color::W, (3 + nxt_pos - 0) % 4 * +1);
              rotate(Color::R, +1);
              rotate(Color::W, (3 + nxt_pos - 0) % 4 * -1);
            }
            break;
          }
        case 5:
          {
            if (edge_ori[pos] == 0) {
              rotate(Color::W, (4 + nxt_pos - 2) % 4 * +1);
              rotate(Color::B, +1);
              rotate(Color::W, (4 + nxt_pos - 2) % 4 * -1);
            } else {
              rotate(Color::W, (5 + nxt_pos - 2) % 4 * +1);
              rotate(Color::R, -1);
              rotate(Color::W, (5 + nxt_pos - 2) % 4 * -1);
            }
            break;
          }
        case 6:
          {
            if (edge_ori[pos] == 0) {
              rotate(Color::W, (4 + nxt_pos - 2) % 4 * +1);
              rotate(Color::B, -1);
              rotate(Color::W, (4 + nxt_pos - 2) % 4 * -1);
            } else {
              rotate(Color::W, (3 + nxt_pos - 2) % 4 * +1);
              rotate(Color::O, +1);
              rotate(Color::W, (3 + nxt_pos - 2) % 4 * -1);
            }
            break;
          }
        case 7:
          {
            if (edge_ori[pos] == 0) {
              rotate(Color::W, (4 + nxt_pos - 0) % 4 * +1);
              rotate(Color::G, +1);
              rotate(Color::W, (4 + nxt_pos - 0) % 4 * -1);
            } else {
              rotate(Color::W, (5 + nxt_pos - 0) % 4 * +1);
              rotate(Color::O, -1);
              rotate(Color::W, (5 + nxt_pos - 0) % 4 * -1);
            }
            break;
          }
        case 8:
        case 9:
        case 10:
        case 11:
          {
            if (edge_ori[pos] == 0) {
              rotate(Color::Y, (12 + nxt_pos - pos) % 4);
              rotate(colors_tmp[nxt_pos], 2);
            } else {
              switch ((12 + nxt_pos - pos) % 4) {
                case 0:
                case 2:
                  {
                    rotate(Color::Y, 1);
                    pos = (pos + 1) % 4 + 8;
                    break;
                  }
              }
              switch ((12 + nxt_pos - pos) % 4) {
                case 1:
                  {
                    rotate(colors_tmp[(pos + 0) % 4], -1);
                    rotate(colors_tmp[(pos + 1) % 4], +1);
                    break;
                  }
                case 3:
                  {
                    rotate(colors_tmp[(pos + 0) % 4], +1);
                    rotate(colors_tmp[(pos + 3) % 4], -1);
                    break;
                  }
              }
            }
            break;
          }
      }
    }

    if (white_edge_to_move_exist == false) {
      break;
    }
  }

  // 2. 흰 면 맞추기
  for (bool white_corner_to_move_exist = false;; white_corner_to_move_exist = false) {
    for (uint8_t pos = 0; pos < 8; ++pos) {
      uint8_t nxt_pos = corner_pos[pos];
      if (nxt_pos != 0 && nxt_pos != 1 && nxt_pos != 2 && nxt_pos != 3) {
        continue;
      }

      if (corner_ori[pos] == 0 && nxt_pos == pos) {
        continue;
      }

      white_corner_to_move_exist = true;

      switch (pos) {
        case 0:
        case 1:
        case 2:
        case 3:
          {
            if (corner_ori[pos] == 0 || corner_ori[pos] == 1) {
              rotate(colors_tmp[(pos + 1) % 4], -1);
              rotate(Color::Y, -1);
              rotate(colors_tmp[(pos + 1) % 4], +1);
              pos = (pos + 3) % 4 + 4;
            } else {
              rotate(colors_tmp[(pos + 0) % 4], +1);
              rotate(Color::Y, +1);
              rotate(colors_tmp[(pos + 0) % 4], -1);
              pos = (pos + 1) % 4 + 4;
            }
            break;
          }
        case 4:
        case 5:
        case 6:
        case 7:
          {
            if (corner_ori[pos] == 0) {
              uint8_t top_pos = (pos + 1) % 4;
              uint8_t top_rotation_count = 0;
              while (corner_pos[top_pos] == 0 || corner_pos[top_pos] == 1 || corner_pos[top_pos] == 2 || corner_pos[top_pos] == 3) {
                rotate(Color::W, 1);
                top_rotation_count++;
              }

              rotate(colors_tmp[(top_pos + 1) % 4], -1);
              rotate(Color::Y, 2);
              rotate(colors_tmp[(top_pos + 1) % 4], +1);

              rotate(Color::W, -top_rotation_count);

              pos = top_pos + 4;
            }

            break;
          }
      }

      switch (pos) {
        case 4:
        case 5:
        case 6:
        case 7:
          {
            if (corner_ori[pos] == 1) {
              while (corner_pos[(nxt_pos + 3) % 4 + 4] != nxt_pos) {
                rotate(Color::Y, 1);
              }

              rotate(colors_tmp[(nxt_pos + 1) % 4], -1);
              rotate(Color::Y, +1);
              rotate(colors_tmp[(nxt_pos + 1) % 4], +1);
            } else if (corner_ori[pos] == 2) {
              while (corner_pos[(nxt_pos + 1) % 4 + 4] != nxt_pos) {
                rotate(Color::Y, 1);
              }

              rotate(colors_tmp[(nxt_pos + 0) % 4], +1);
              rotate(Color::Y, -1);
              rotate(colors_tmp[(nxt_pos + 0) % 4], -1);
            }
          }
      }

      break;
    }

    if (white_corner_to_move_exist == false) {
      break;
    }
  }

  // 3. 두 층 맞추기
  for (bool side_edge_to_move_exist = false;; side_edge_to_move_exist = false) {
    for (int8_t pos = 11; pos >= 0; --pos) {
      uint8_t nxt_pos = edge_pos[pos];
      if (nxt_pos != 4 && nxt_pos != 5 && nxt_pos != 6 && nxt_pos != 7) {
        continue;
      }

      if (edge_ori[pos] == 0 && nxt_pos == pos) {
        continue;
      }

      side_edge_to_move_exist = true;

      switch (pos) {
        case 0:
        case 1:
        case 2:
        case 3:
          {
            break;
          }
        case 4:
        case 5:
        case 6:
        case 7:
          {
            twist_rhand(Color::Y, colors_tmp[(pos + 0) % 4]);
            twist_lhand(Color::Y, colors_tmp[(pos + 1) % 4]);
            break;
          }
        case 8:
        case 9:
        case 10:
        case 11:
          {
            constexpr uint8_t mid_poses[2][4] = {
              { 11, 11, 9, 9 },
              { 10, 8, 8, 10 },
            };
            uint8_t mid_pos = mid_poses[edge_ori[pos]][nxt_pos - 4];
            rotate(Color::Y, (4 + mid_pos - pos) % 4);

            if ((8 + nxt_pos - mid_pos) % 4 == 1) {
              twist_lhand(Color::Y, colors_tmp[(nxt_pos + 1) % 4]);
              twist_rhand(Color::Y, colors_tmp[(nxt_pos + 0) % 4]);
            } else if ((8 + nxt_pos - mid_pos) % 4 == 2) {
              twist_rhand(Color::Y, colors_tmp[(nxt_pos + 0) % 4]);
              twist_lhand(Color::Y, colors_tmp[(nxt_pos + 1) % 4]);
            }
            break;
          }
      }
    }

    if (side_edge_to_move_exist == false) {
      break;
    }
  }

  // 4. 노란 십자가 맞추기
  {
    if (edge_ori[8] == 0 && edge_ori[9] == 0 && edge_ori[10] == 0 && edge_ori[11] == 0)
      ;                                                                                         // 행복한 경우
    else if (edge_ori[8] == 1 && edge_ori[9] == 1 && edge_ori[10] == 1 && edge_ori[11] == 1) {  // .
      rotate(Color::G, +1);
      twist_rhand(Color::Y, Color::O);
      rotate(Color::G, -1);
    } else if (edge_ori[8] == 0 && edge_ori[10] == 0) {  // |
      rotate(Color::O, +1);
      twist_rhand(Color::Y, Color::B);
      rotate(Color::O, -1);
    } else if (edge_ori[9] == 0 && edge_ori[11] == 0) {  // -
      rotate(Color::G, +1);
      twist_rhand(Color::Y, Color::O);
      rotate(Color::G, -1);
    }

    if (edge_ori[8] == 0 && edge_ori[9] == 0 && edge_ori[10] == 0 && edge_ori[11] == 0)
      ;     // 행복한 경우
    else {  // ㄱ
      for (uint8_t i = 0; i < 4; ++i) {
        if (edge_ori[8 + (i + 0) % 4] == 0 && edge_ori[8 + (i + 1) % 4] == 0) {
          rotate(colors_tmp[(i + 3) % 4], +1);
          twist_rhand(Color::Y, colors_tmp[(i + 2) % 4]);
          twist_rhand(Color::Y, colors_tmp[(i + 2) % 4]);
          rotate(colors_tmp[(i + 3) % 4], -1);
          break;
        }
      }
    }
  }

  // 5. 노란 코너 위치 맞추기
  for (bool yello_face_will_rotate = true, yello_face_rotated = false;;
       yello_face_will_rotate = true, yello_face_rotated = false) {
    for (uint8_t i = 0; i < 4; ++i, rotate(Color::Y, 1)) {
      if (corner_pos[4] == 4 && corner_pos[5] == 5 && corner_pos[6] == 6 && corner_pos[7] == 7) {  // 행복한 경우
        yello_face_will_rotate = false;
        break;
      }

      for (uint8_t j = 0; j < 4; ++j) {
        if (corner_pos[4 + (j + 0) % 4] == 4 + (j + 0) % 4 && corner_pos[4 + (j + 1) % 4] == 4 + (j + 1) % 4) {
          Color twist_r_side = colors_tmp[(j + 3) % 4];
          twist_rhand(Color::Y, twist_r_side);
          twist_rhand(Color::Y, twist_r_side);
          twist_rhand(Color::Y, twist_r_side);
          Color twist_l_side = colors_tmp[j];
          twist_lhand(Color::Y, twist_l_side);
          twist_lhand(Color::Y, twist_l_side);
          twist_lhand(Color::Y, twist_l_side);

          yello_face_rotated = true;
          break;
        }
      }

      if (yello_face_rotated == true) {
        break;
      }
    }

    if (yello_face_will_rotate == false) {
      break;
    }
    if (yello_face_rotated == false) {
      twist_rhand(Color::Y, Color::O);
      twist_rhand(Color::Y, Color::O);
      twist_rhand(Color::Y, Color::O);
      twist_lhand(Color::Y, Color::G);
      twist_lhand(Color::Y, Color::G);
      twist_lhand(Color::Y, Color::G);
    }
  }

  // 6. 노란 면 맞추기
  for (uint8_t i = 0; i < 4; ++i) {
    while (!(corner_pos[4] == 4 + i && corner_ori[4] == 0)) {
      twist_rhand(Color::W, Color::R);
    }
    rotate(Color::Y, -1);
  }

  // 7. 전체 맞추기
  if (edge_pos[8] == 8 && edge_pos[9] == 9 && edge_pos[10] == 10 && edge_pos[11] == 11)
    ;  // 행복한 경우
  else {
    if (edge_pos[8] != 8 && edge_pos[9] != 9 && edge_pos[10] != 10 && edge_pos[11] != 11) {  // 행복한 경우
      twist_rhand(Color::Y, Color::O);
      twist_lhand(Color::Y, Color::R);
      twist_rhand(Color::Y, Color::O);
      twist_rhand(Color::Y, Color::O);
      twist_rhand(Color::Y, Color::O);
      twist_rhand(Color::Y, Color::O);
      twist_rhand(Color::Y, Color::O);
      twist_lhand(Color::Y, Color::R);
      twist_lhand(Color::Y, Color::R);
      twist_lhand(Color::Y, Color::R);
      twist_lhand(Color::Y, Color::R);
      twist_lhand(Color::Y, Color::R);
    }
    for (uint8_t i = 0; i < 4; ++i) {
      if (edge_pos[8 + i] == 8 + i) {
        Color twist_r_side = colors_tmp[(i + 3) % 4];
        Color twist_l_side = colors_tmp[(i + 1) % 4];
        if (edge_pos[8 + (2 + i) % 4] == 8 + (3 + i) % 4) {
          twist_lhand(Color::Y, twist_l_side);
          twist_rhand(Color::Y, twist_r_side);
          twist_lhand(Color::Y, twist_l_side);
          twist_lhand(Color::Y, twist_l_side);
          twist_lhand(Color::Y, twist_l_side);
          twist_lhand(Color::Y, twist_l_side);
          twist_lhand(Color::Y, twist_l_side);
          twist_rhand(Color::Y, twist_r_side);
          twist_rhand(Color::Y, twist_r_side);
          twist_rhand(Color::Y, twist_r_side);
          twist_rhand(Color::Y, twist_r_side);
          twist_rhand(Color::Y, twist_r_side);
        } else if (edge_pos[8 + (2 + i) % 4] == 8 + (1 + i) % 4) {
          twist_rhand(Color::Y, twist_r_side);
          twist_lhand(Color::Y, twist_l_side);
          twist_rhand(Color::Y, twist_r_side);
          twist_rhand(Color::Y, twist_r_side);
          twist_rhand(Color::Y, twist_r_side);
          twist_rhand(Color::Y, twist_r_side);
          twist_rhand(Color::Y, twist_r_side);
          twist_lhand(Color::Y, twist_l_side);
          twist_lhand(Color::Y, twist_l_side);
          twist_lhand(Color::Y, twist_l_side);
          twist_lhand(Color::Y, twist_l_side);
          twist_lhand(Color::Y, twist_l_side);
        }
        break;
      }
    }
  }

  // 8. 중복 회전 정리(WWWW >> void, WWW >> w)
  for (uint16_t i = 0; i < container.length() - 3; ++i) {
    if (container[i] == container[i + 1] && container[i] == container[i + 2] && container[i] == container[i + 3]) {
      container[i + 0] = ' ';
      container[i + 1] = ' ';
      container[i + 2] = ' ';
      container[i + 3] = ' ';
    }
  }
  for (uint16_t i = 0; i < container.length() - 2; ++i) {
    if (container[i] == container[i + 1] && container[i] == container[i + 2]) {
      switch (container[i]) {
        case 'W':
          container[i] = 'w';
          container[i + 1] = ' ';
          container[i + 2] = ' ';
          break;
        case 'Y':
          container[i] = 'y';
          container[i + 1] = ' ';
          container[i + 2] = ' ';
          break;
        case 'G':
          container[i] = 'g';
          container[i + 1] = ' ';
          container[i + 2] = ' ';
          break;
        case 'B':
          container[i] = 'b';
          container[i + 1] = ' ';
          container[i + 2] = ' ';
          break;
        case 'O':
          container[i] = 'o';
          container[i + 1] = ' ';
          container[i + 2] = ' ';
          break;
        case 'R':
          container[i] = 'r';
          container[i + 1] = ' ';
          container[i + 2] = ' ';
          break;

        case 'w':
          container[i] = 'W';
          container[i + 1] = ' ';
          container[i + 2] = ' ';
          break;
        case 'y':
          container[i] = 'Y';
          container[i + 1] = ' ';
          container[i + 2] = ' ';
          break;
        case 'g':
          container[i] = 'G';
          container[i + 1] = ' ';
          container[i + 2] = ' ';
          break;
        case 'b':
          container[i] = 'B';
          container[i + 1] = ' ';
          container[i + 2] = ' ';
          break;
        case 'o':
          container[i] = 'O';
          container[i + 1] = ' ';
          container[i + 2] = ' ';
          break;
        case 'r':
          container[i] = 'R';
          container[i + 1] = ' ';
          container[i + 2] = ' ';
          break;
      }
    }
  }
}

#endif // !VIRTUAL_CUBE_H