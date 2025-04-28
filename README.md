# DLL Injector
## Actual version: v0.2.6 (28.04.2025)

An application with its own simple API for implementing injectors for DLLs.

Written in `C++`.

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