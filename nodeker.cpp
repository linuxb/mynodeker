//============================================================================
// Name        : uvtest.cpp
// Author      : linuxb
// Version     :
// Copyright   : LinuxbCoroperation
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "mykernel.h"
using namespace std;

int main() {
	//event main loop
	uv_loop_t* loop = uv_default_loop();
	///
	std::cout << "looper quiting....\n!" <<std::endl;
	uv_run(loop,UV_RUN_DEFAULT);
	return 0;
}
