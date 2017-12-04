#include <iostream>
int Factorial(int a) {
	return (a == 1) ? 1 : Factorial(a - 1)*a;
}

#Function testing

int main() {
	int x = 12;
	std::cout << Factorial(x) << std::endl;
}
