#include <iostream>
#include "injector.h"

using namespace std;

int main()
{
	string tempStr;
	Injector inj;

	string dllPath = "";
	DWORD pid = -1;

	cout << "DllInjector by " << endl;

	while (!OpenProcess(PROCESS_ALL_ACCESS, false, pid)) {
		cout << "Process ID: " << endl << ">> ";
		cin >> pid;
		cout << endl;
	}

	while (!inj.checkDllFile(dllPath.c_str())) {
		cout << "dllPath: " << endl << ">> ";
		cin >> dllPath;
		cout << endl;
	}

	if (!inj.inject(pid, dllPath.c_str())) {
		cerr << "Failed to inject process." << endl;
		return -1;
	}

	cerr << "The injection was successful." << endl;

	return 0;	
}