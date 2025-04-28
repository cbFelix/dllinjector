#include <iostream>

#include "injector.h"

#define INJECTOR_H

using namespace std;

int main() {
	try {
		DWORD processId = InjectorAPI::FindProcessID(L"notepad.exe");
		
		const char* path = "F:/dllTest.dll";

		InjectorAPI::DllValidator val(path);

		if (!val.isValidDLL()) {
			return -1;
		}
	}
	catch (exception &e) {
		cerr << "Error: " << e.what() << endl;
		return -1;
	}
	
	return 0;
}