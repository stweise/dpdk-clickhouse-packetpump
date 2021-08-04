#include <iostream>
extern "C" void f(int);
void f(int i)
{
	std::cout << "Hello " << i << std::endl;
}
