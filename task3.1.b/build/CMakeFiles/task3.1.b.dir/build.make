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
CMAKE_SOURCE_DIR = F:\gitclone\bupt_tasks\task3.1.b

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = F:\gitclone\bupt_tasks\task3.1.b\build

# Include any dependencies generated for this target.
include CMakeFiles/task3.1.b.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/task3.1.b.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/task3.1.b.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/task3.1.b.dir/flags.make

CMakeFiles/task3.1.b.dir/main.cpp.obj: CMakeFiles/task3.1.b.dir/flags.make
CMakeFiles/task3.1.b.dir/main.cpp.obj: F:/gitclone/bupt_tasks/task3.1.b/main.cpp
CMakeFiles/task3.1.b.dir/main.cpp.obj: CMakeFiles/task3.1.b.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=F:\gitclone\bupt_tasks\task3.1.b\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/task3.1.b.dir/main.cpp.obj"
	C:\PROGRA~2\mingw64\bin\G__~1.EXE $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/task3.1.b.dir/main.cpp.obj -MF CMakeFiles\task3.1.b.dir\main.cpp.obj.d -o CMakeFiles\task3.1.b.dir\main.cpp.obj -c F:\gitclone\bupt_tasks\task3.1.b\main.cpp

CMakeFiles/task3.1.b.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/task3.1.b.dir/main.cpp.i"
	C:\PROGRA~2\mingw64\bin\G__~1.EXE $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E F:\gitclone\bupt_tasks\task3.1.b\main.cpp > CMakeFiles\task3.1.b.dir\main.cpp.i

CMakeFiles/task3.1.b.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/task3.1.b.dir/main.cpp.s"
	C:\PROGRA~2\mingw64\bin\G__~1.EXE $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S F:\gitclone\bupt_tasks\task3.1.b\main.cpp -o CMakeFiles\task3.1.b.dir\main.cpp.s

# Object files for target task3.1.b
task3_1_b_OBJECTS = \
"CMakeFiles/task3.1.b.dir/main.cpp.obj"

# External object files for target task3.1.b
task3_1_b_EXTERNAL_OBJECTS =

task3.1.b.exe: CMakeFiles/task3.1.b.dir/main.cpp.obj
task3.1.b.exe: CMakeFiles/task3.1.b.dir/build.make
task3.1.b.exe: CMakeFiles/task3.1.b.dir/linkLibs.rsp
task3.1.b.exe: CMakeFiles/task3.1.b.dir/objects1.rsp
task3.1.b.exe: CMakeFiles/task3.1.b.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=F:\gitclone\bupt_tasks\task3.1.b\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable task3.1.b.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\task3.1.b.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/task3.1.b.dir/build: task3.1.b.exe
.PHONY : CMakeFiles/task3.1.b.dir/build

CMakeFiles/task3.1.b.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\task3.1.b.dir\cmake_clean.cmake
.PHONY : CMakeFiles/task3.1.b.dir/clean

CMakeFiles/task3.1.b.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" F:\gitclone\bupt_tasks\task3.1.b F:\gitclone\bupt_tasks\task3.1.b F:\gitclone\bupt_tasks\task3.1.b\build F:\gitclone\bupt_tasks\task3.1.b\build F:\gitclone\bupt_tasks\task3.1.b\build\CMakeFiles\task3.1.b.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/task3.1.b.dir/depend

