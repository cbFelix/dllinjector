#include <iostream>

#include "injector.h"

#define INJECTOR_H

using namespace std;

int main() {
	try {
		cout << "DLLInjector v0.3.1" << endl;
		cout << "Github: https://github.com/cbFelix/dllinjector" << endl;
		cout << "\nWarning! The current version is in development and has not been tested. Use at your own risk. \nOf course, the probability of this is very small, at least there were no problems during test runs.\n" << endl;

		string buffer = "";
		cout << "Process name: ";
		cin >> buffer;

		DWORD processId = InjectorAPI::FindProcessID(wstring(buffer.begin(), buffer.end()));

		InjectorAPI::Injector inj(processId);

		cout << "Path to dll: ";
		cin >> buffer;

		inj.InjectNt(buffer.c_str());
		
		cout << "Injection successful." << endl;
	}
	catch (exception &e) {
		cerr << "Error: " << e.what() << endl;
		return -1;
	}
	
	return 0;
}