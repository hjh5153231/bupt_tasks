# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.27

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = F:\gitclone\bupt_tasks\task1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = F:\gitclone\bupt_tasks\task1\build

# Include any dependencies generated for this target.
include CMakeFiles/main.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/main.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/main.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/main.dir/flags.make

CMakeFiles/main.dir/src/Calculator.obj: CMakeFiles/main.dir/flags.make
CMakeFiles/main.dir/src/Calculator.obj: CMakeFiles/main.dir/includes_CXX.rsp
CMakeFiles/main.dir/src/Calculator.obj: F:/gitclone/bupt_tasks/task1/src/Calculator.cpp
CMakeFiles/main.dir/src/Calculator.obj: CMakeFiles/main.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\gitclone\bupt_tasks\task1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/main.dir/src/Calculator.obj"
	C:\PROGRA~2\mingw64\bin\G__~1.EXE $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/main.dir/src/Calculator.obj -MF CMakeFiles\main.dir\src\Calculator.obj.d -o CMakeFiles\main.dir\src\Calculator.obj -c F:\gitclone\bupt_tasks\task1\src\Calculator.cpp

CMakeFiles/main.dir/src/Calculator.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/main.dir/src/Calculator.i"
	C:\PROGRA~2\mingw64\bin\G__~1.EXE $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E F:\gitclone\bupt_tasks\task1\src\Calculator.cpp > CMakeFiles\main.dir\src\Calculator.i

CMakeFiles/main.dir/src/Calculator.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/main.dir/src/Calculator.s"
	C:\PROGRA~2\mingw64\bin\G__~1.EXE $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S F:\gitclone\bupt_tasks\task1\src\Calculator.cpp -o CMakeFiles\main.dir\src\Calculator.s

CMakeFiles/main.dir/main.obj: CMakeFiles/main.dir/flags.make
CMakeFiles/main.dir/main.obj: CMakeFiles/main.dir/includes_CXX.rsp
CMakeFiles/main.dir/main.obj: F:/gitclone/bupt_tasks/task1/main.cpp
CMakeFiles/main.dir/main.obj: CMakeFiles/main.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\gitclone\bupt_tasks\task1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/main.dir/main.obj"
	C:\PROGRA~2\mingw64\bin\G__~1.EXE $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/main.dir/main.obj -MF CMakeFiles\main.dir\main.obj.d -o CMakeFiles\main.dir\main.obj -c F:\gitclone\bupt_tasks\task1\main.cpp

CMakeFiles/main.dir/main.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/main.dir/main.i"
	C:\PROGRA~2\mingw64\bin\G__~1.EXE $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E F:\gitclone\bupt_tasks\task1\main.cpp > CMakeFiles\main.dir\main.i

CMakeFiles/main.dir/main.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/main.dir/main.s"
	C:\PROGRA~2\mingw64\bin\G__~1.EXE $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S F:\gitclone\bupt_tasks\task1\main.cpp -o CMakeFiles\main.dir\main.s

# Object files for target main
main_OBJECTS = \
"CMakeFiles/main.dir/src/Calculator.obj" \
"CMakeFiles/main.dir/main.obj"

# External object files for target main
main_EXTERNAL_OBJECTS =

main.exe: CMakeFiles/main.dir/src/Calculator.obj
main.exe: CMakeFiles/main.dir/main.obj
main.exe: CMakeFiles/main.dir/build.make
main.exe: CMakeFiles/main.dir/linkLibs.rsp
main.exe: CMakeFiles/main.dir/objects1.rsp
main.exe: CMakeFiles/main.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=F:\gitclone\bupt_tasks\task1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable main.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\main.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/main.dir/build: main.exe
.PHONY : CMakeFiles/main.dir/build

CMakeFiles/main.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\main.dir\cmake_clean.cmake
.PHONY : CMakeFiles/main.dir/clean

CMakeFiles/main.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" F:\gitclone\bupt_tasks\task1 F:\gitclone\bupt_tasks\task1 F:\gitclone\bupt_tasks\task1\build F:\gitclone\bupt_tasks\task1\build F:\gitclone\bupt_tasks\task1\build\CMakeFiles\main.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/main.dir/depend

