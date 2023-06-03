# CMake C++ Project Template
*cmake_cpp_project_template* is a simple template for C and C++ projects.
The template includes targets for building libraries, tests, and executables.

Currently supported build on GNU/Linux.

The *cmake_cpp_project_template* assumes you want to setup a project using
* CMake (3.0 or above)
* C/C++ compiler


## Contents

* [Usage](#usage)
* [Targets](#targets)
  * [Libraries](#libraries)
  * [Executables](#executables)
  * [Tests](#tests)
  * [Custom targets](#custom-targets)
  * [Drivers](#drivers)
  * [Externals](#externals)
* [Build](#build)
* [License](#license)

## Usage

Template Usage:
* the template is added to the project by a submodule/copied in the root directory;
* files `CMakeLists.txt.in`, `sources.cmake.in` and `Makefile.in` are copied to the root directory;
* rename `CMakeLists.txt.in` -> `CMakeLists.txt`, `sources.cmake.in` -> `sources.cmake` and `Makefile.in` -> `Makefile`;
* in the copied `CMakeLists.txt` file, need to replace `@project_name@` with the name of the project being developed and `@common_cmake_dir@` with the name of the submodule/copy оf template directory;
* in the copied `Makefile` file, need to replace `@common_cmake_dir@` with the name of the submodule/copy оf template directory.

## Targets

The library supports targets:
* [LibTarget](#libraries) - for building libraries;
* [ExecTarget](#executables) - for building executable files;
* [TestTarget](#tests) - for building tests;
* [DriverTarget](#drivers) - for building kernel modules;
* [ExternalTarget and WrapperTarget](#externals) - for building external modules.

### Libraries

### Executables

### Tests

### Custom targets

### Drivers

### Externals

## Build

## License

&copy; 2022 Chistyakov Alexander.

Open sourced under MIT license, the terms of which can be read here — [MIT License](http://opensource.org/licenses/MIT).

