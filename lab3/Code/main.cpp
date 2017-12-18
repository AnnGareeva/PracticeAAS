#include "thread.h"

#define MY_DEF_USE_LIBTAP
#ifdef MY_DEF_USE_LIBTAP

#define TAP_COMPILE
#include "libtap\cpp_tap.h"

using namespace std;

int main(int, char *[]) {
  //
  plan_tests(8);
  double arr[100][2];
  //
  arr[0][0] = 0.0;
  arr[0][1] = 0.0;
  arr[1][0] = 3.0;
  arr[1][1] = 0.0;
  ok(LenPoints(arr[0][0], arr[1][0], arr[0][1], arr[1][1]) == 3.0, "TestLenPoints1");
  //
  arr[0][0] = 10.0;
  arr[0][1] = 0.0;
  arr[1][0] = -10.0;
  arr[1][1] = 0.0;
  ok(LenPoints(arr[0][0], arr[1][0], arr[0][1], arr[1][1]) == 20.0, "TestLenPoints2");
  //
  arr[0][0] = 0.0;
  arr[0][1] = 0.0;
  arr[1][0] = 2.0;
  arr[1][1] = 0.0;
  arr[2][0] = 2.0;
  arr[2][1] = 2.0;
  arr[3][0] = 0.0;
  arr[3][1] = 2.0;
  ok(LenThread(4, 1, arr) == 14.28, "Test from timus");
  //
  arr[0][0] = 0.0;
  arr[0][1] = 0.0;
  arr[1][0] = 3.0;
  arr[1][1] = 0.0;
  arr[2][0] = 3.0;
  arr[2][1] = 3.0;
  ok(LenThread(3, 2, arr) == 22.80, "Test 1");
  //
  ok(LenThread(0, 2, arr) == -2, "Test 2");
  //
  ok(LenThread(-10, 3, arr) == -2, "Test 3");
  //
  arr[0][0] = 0.0;
  arr[0][1] = 0.0;
  ok(LenThread(1, 3, arr) == 18.84, "Test 4");
  //
  for(int i(0); i < 99; i++) {
    arr[i][0] = i;
    arr[i][1] = i;
  }
  arr[99][0] = 50.0;
  arr[99][1] = 99.0;
  ok(LenThread(100, 1, arr) == 303.79, "Test 5");
  //
  return exit_status();
  //
  return 0;
}

#endif
