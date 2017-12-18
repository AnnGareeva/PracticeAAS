#include <iostream>
#include <vector>

using namespace std;

int main() {
  vector<long long> values;
  vector<long long> test;
  long long num = -1;
  if (num == 0) {
    return 1;
  }
  cin >> num;
  while (num != 0) {
    test.push_back(num);
    cin >> num;
  }
  system("cls");
  for (long long i = 0; i < test.size(); i++) {
    values.push_back(0);
    values.push_back(1);
    for (long long j = 2; j < (test[i] + 1); j++) {
      long long index = 0;
      if (j % 2 == 0) {
        index = j / 2;
        values.push_back(values[index]);
      } else {
        index = (j - 1) / 2;
        values.push_back(values[index] + values[index + 1]);
      }
    }
    long long maximum = 0;
    for (int j = 0; j < values.size(); j++) {
      if (maximum < values[j]) maximum = values[j];
    }
    cout << maximum;
    if (i != (test.size() - 1)) {
      cout << endl;
    }
    values.clear();
  }
  system("pause");
  return 0;
}
