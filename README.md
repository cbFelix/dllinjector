# DLL Injector by keww:L

## Overview

The **DLL Injector** is a tool that allows you to inject a dynamic link library (DLL) into a running process. The tool supports injection into both 32-bit and 64-bit processes on Windows. This project is intended for **educational purposes** only.

## Features
- Injects DLLs into 32-bit and 64-bit processes.
- Uses both `NtCreateThreadEx` and `CreateRemoteThread` for DLL injection.
- Automatically detects the architecture (32-bit or 64-bit) of the target process.
- Supports temporary folder creation and cleanup of the DLL.
- Simple command-line interface for choosing the process and DLL.

## How to Download and Run

### 1. Download the Release
You can download the pre-built executable from the [Releases](https://github.com/cbFelix/dllinjector/releases/tag/dllinjector) section on GitHub. 

- **dllinjector.exe**: This is the compiled version of the project for easy use.
  
### 2. Running the Executable
1. After downloading, extract the archive (if applicable).
2. Run the `dllinjector.exe` file.
3. Follow the instructions in the console window:
   - Enter the **Process ID** of the target process.
   - Provide the **absolute path to the DLL** you want to inject.

### 3. Injecting the DLL
Once you provide the required inputs, the tool will inject the DLL into the specified process.

## How to Compile the Project

### Prerequisites
To compile the project from source, ensure you have the following installed:
- **Visual Studio** (or any C++ compiler that supports Windows API)
- **Windows SDK**

### Steps to Compile:
1. Clone the repository:
   "https://github.com/cbFelix/dllinjector.git"

2. Open the solution file (`.sln`) in **Visual Studio**.

3. Build the project:
   - Select either `x86` (32-bit) or `x64` (64-bit) build configuration based on your target architecture.
   - Click **Build** -> **Build Solution** (or press `Ctrl + Shift + B`).

4. The compiled executable (`dllinjector.exe`) will be available in the output directory (`/Release` or `/Debug`).

## How to Use the Program

1. Launch the executable:
   "dllinjector.exe"

2. Enter the **Process ID** of the application you want to inject the DLL into.
   - You can use Task Manager or tools like **Process Explorer** to find the Process ID.

3. Provide the full path to the **DLL** you want to inject.

4. The tool will handle the rest, and you should see messages indicating success or failure of the injection.

## Educational Purpose Disclaimer

This project is intended **strictly for educational purposes**. The developer does **not take responsibility** for any misuse of this tool. Please ensure that you use it responsibly and within the bounds of the law.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Troubleshooting

- **Antivirus Issues**: Some antivirus programs might flag the tool as suspicious. This is a common issue with DLL injectors. If you are sure the tool is safe, you can create an exception in your antivirus software.
  
- **Process Architecture Mismatch**: Ensure that the architecture (32-bit or 64-bit) of the DLL matches that of the target process. Otherwise, the injection will fail.

- **Administrator Privileges**: In some cases, you may need to run the tool with administrator privileges to inject into certain processes.
