# DLL Injector
## Actual version: v0.3.1 (28.04.2025)

An application with its own simple API for implementing injectors for DLLs.

Written in `C++`.

## Antiviruses
**Important:** Antiviruses may mistakenly mark the injector as malicious. This happens due to the methods of working with process memory. 
### To use:
- Add the file to the antivirus exceptions.
- Disable the antivirus for the duration of testing (only in an isolated environment!).
- If you are still not satisfied with something, you can build the injector yourself from the sources. Otherwise, just do not use this injector.
---
### Current features:
- A ready-made simple dll injector.
- A simple DLL file validator.
- A couple of functions for working with processes.

### Goals:
- [ ] Create a modular injector for system extensibility and flexibility.
- [ ] Digital signature verification.
- [ ] Functions for signing dll.
- [x] Injection via NtCreateThreadEx (Task closed 04/28/2025).
- [ ] Injection via Manual Mapping.
- [ ] Graphical user interface.

## Examples
You can test some of the features using the built-in examples. 
For now, there is only one example file: `example.cpp`. This is the file used in the current build.