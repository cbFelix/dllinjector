#include <iostream>

#include "injector.h"

#define INJECTOR_H

using namespace std;

int main() {
	try {
		DWORD processId = InjectorAPI::FindProcessID(L"notepad.exe");

	}
	catch (exception &e) {
		cerr << "Error: " << e.what() << endl;
		return -1;
	}
	
	return 0;
}