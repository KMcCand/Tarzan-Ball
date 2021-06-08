# List of demo programs
DEMOS = tarzan-ball level_maker level_viewer
# List of C files in "libraries" that we provide
STAFF_LIBS = test_util sdl_wrapper
# List of C files in "libraries" that you will write.
# This also defines the order in which the tests are run.
STUDENT_LIBS = vector list polygon color image my_aux body scene forces collision textbox

# If we're not on Windows...
ifneq ($(OS), Windows_NT)

# Use clang as the C compiler
CC = clang
# Flags to pass to clang:
# -Iinclude tells clang to look for #include files in the "include" folder
# -Wall turns on all warnings
# -g adds filenames and line numbers to the executable for useful stack traces
# -fno-omit-frame-pointer allows stack traces to be generated
#   (take CS 24 for a full explanation)
# -fsanitize=address enables asan
CFLAGS = -Iinclude $(shell sdl2-config --cflags | sed -e "s/include\/SDL2/include/") -Wall -g -fno-omit-frame-pointer -fsanitize=address -Wno-nullability-completeness
# Compiler flag that links the program with the math library
LIB_MATH = -lm
# Compiler flags that link the program with the math and SDL libraries.
# Note that $(...) substitutes a variable's value, so this line is equivalent to
# LIBS = -lm -lSDL2 -lSDL2_gfx
LIBS = $(LIB_MATH) $(shell sdl2-config --libs) -lSDL2_gfx -lSDL2_ttf -lSDL2_Image

# List of compiled .o files corresponding to STUDENT_LIBS, e.g. "out/vector.o".
# Don't worry about the syntax; it's just adding "out/" to the start
# and ".o" to the end of each value in STUDENT_LIBS.
STUDENT_OBJS = $(addprefix out/,$(STUDENT_LIBS:=.o))
# List of test suite executables, e.g. "bin/test_suite_vector"
TEST_BINS = $(addprefix bin/test_suite_,$(STUDENT_LIBS))
# List of demo executables, i.e. "bin/bounce".
DEMO_BINS = $(addprefix bin/,$(DEMOS))
# All executables (the concatenation of TEST_BINS and DEMO_BINS)
BINS = $(TEST_BINS) $(DEMO_BINS)

# The first Make rule. It is relatively simple:
# "To build 'all', make sure all files in BINS are up to date."
# You can execute this rule by running the command "make all", or just "make".
all: $(BINS)

# Any .o file in "out" is built from the corresponding C file.
# Although .c files can be directly compiled into an executable, first building
# .o files reduces the amount of work needed to rebuild the executable.
# For example, if only list.c was modified since the last build, only list.o
# gets recompiled, and clang reuses the other .o files to build the executable.
#
# "%" means "any string".
# Unlike "all", this target has a build command.
# "$^" is a special variable meaning "the source files"
# and $@ means "the target file", so the command tells clang
# to compile the source C file into the target .o file.
out/%.o: library/%.c # source file may be found in "library"
	$(CC) -c $(CFLAGS) $^ -o $@
out/%.o: demo/%.c # or "demo"
	$(CC) -c $(CFLAGS) $^ -o $@
out/%.o: tests/%.c # or "tests"
	$(CC) -c $(CFLAGS) $^ -o $@

# Builds bin/bounce by linking the necessary .o files.
# Unlike the out/%.o rule, this uses the LIBS flags and omits the -c flag,
# since it is building a full executable.
bin/bounce: out/bounce.o out/sdl_wrapper.o $(STUDENT_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

bin/gravity: out/gravity.o out/sdl_wrapper.o $(STUDENT_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

bin/pacman: out/pacman.o out/sdl_wrapper.o $(STUDENT_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

bin/nbodies: out/nbodies.o out/sdl_wrapper.o $(STUDENT_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

bin/damping: out/damping.o out/sdl_wrapper.o $(STUDENT_OBJS)
		$(CC) $(CFLAGS) $(LIBS) $^ -o $@

bin/spaceinvaders: out/spaceinvaders.o out/sdl_wrapper.o $(STUDENT_OBJS)
		$(CC) $(CFLAGS) $(LIBS) $^ -o $@

bin/pegs: out/pegs.o out/sdl_wrapper.o $(STUDENT_OBJS)
		$(CC) $(CFLAGS) $(LIBS) $^ -o $@

bin/brickbreaker: out/brickbreaker.o out/sdl_wrapper.o $(STUDENT_OBJS)
		$(CC) $(CFLAGS) $(LIBS) $^ -o $@

bin/tongue: out/tongue.o out/sdl_wrapper.o $(STUDENT_OBJS)
		$(CC) $(CFLAGS) $(LIBS) $^ -o $@

bin/level_maker: out/level_maker.o out/sdl_wrapper.o $(STUDENT_OBJS)
		$(CC) $(CFLAGS) $(LIBS) $^ -o $@

bin/level_viewer: out/level_viewer.o out/sdl_wrapper.o $(STUDENT_OBJS)
		$(CC) $(CFLAGS) $(LIBS) $^ -o $@

bin/tarzan-ball: out/tarzan-ball.o out/sdl_wrapper.o $(STUDENT_OBJS)
		$(CC) $(CFLAGS) $(LIBS) $^ -o $@

# Builds the test suite executables from the corresponding test .o file
# and the library .o files. The only difference from the demo build command
# is that it doesn't link the SDL libraries.
bin/test_suite_%: out/test_suite_%.o out/test_util.o $(STUDENT_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@


bin/student_tests: out/student_tests.o out/test_util.o $(STUDENT_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

# Runs the tests. "$(TEST_BINS)" requires the test executables to be up to date.
# The command is a simple shell script:
# "set -e" configures the shell to exit if any of the tests fail
# "for f in $(TEST_BINS); do ...; done" loops over the test executables,
#    assigning the variable f to each one
# "echo $$f" prints the test suite
# "$$f" runs the test; "$$" escapes the $ character,
#   and "$f" tells the shell to substitute the value of the variable f
# "echo" prints a newline after each test's output, for readability
test: $(TEST_BINS)
	set -e; for f in $(TEST_BINS); do echo $$f; $$f; echo; done

# Removes all compiled files.
# find <dir> is the command to find files in a directory
# ! -name .gitignore tells find to ignore the .gitignore
# -type f only finds files
# -delete deletes all the files found
clean:
	find out/ ! -name .gitignore -type f -delete && \
	find bin/ ! -name .gitignore -type f -delete

# This special rule tells Make that "all", "clean", and "test" are rules
# that don't build a file.
.PHONY: all clean test
# Tells Make not to delete the .o files after the executable is built
.PRECIOUS: out/%.o































# Windows is _special_
# Define a completely separate set of rules, because syntax and shell
else

# Make normally uses sh, which isn't on Windows by default
# Some systems might have it though...
# So, explicitly use cmd (ew)
# I want to rewrite the test and clean portions in Powershell but
# don't have the time now.
SHELL = cmd.exe

# Use MSVC cl.exe as the C compiler
CC = cl.exe

# Flags to pass to cl.exe:
# -I"C:/Users/$(USERNAME)/msvc/include"
#	- include files that would normally be in /usr/include or something
# -Iinclude = -Iinclude
# -Zi = -g (with debug info in a separate file)
# -W3 turns on warnings (W4 is overkill for this class)
# -Oy- = -fno-omit-frame-pointer. May be unnecessary.
# -fsanitize=address = ...
CFLAGS := -I"C:/Users/$(USERNAME)/msvc/include"
CFLAGS += -Iinclude -Zi -W3 -Oy-
# You may want to turn this off for certain types of debugging.
CFLAGS += -fsanitize=address

# Define _WIN32, telling the programs that they are running on Windows.
CFLAGS += -D_WIN32
# Math constants are not in the standard
CFLAGS += -D_USE_MATH_DEFINES
# Some functions are """unsafe""", like snprintf. We don't care.
CFLAGS += -D_CRT_SECURE_NO_WARNINGS
# Include the full path for the msCompile problem matcher
C_FLAGS += -FC

# Libraries that we are linking against.
# Note that a lot of the base Windows ones are missing - the
# libraries I've distributed are _dynamically linked_, because otherwise,
# we'd need to manually link a lot of crap.
LIBS = SDL2main.lib SDL2.lib SDL2_gfx.lib shell32.lib

# Tell cl to look for lib files in this folder
LINKEROPTS = -LIBPATH:"C:/Users/$(USERNAME)/msvc/lib"
# If SDL2 is included in a file with main, it takes over main with its own def.
# We need to explicitly indicate the application type.
# NOTE: CONSOLE is single-threaded. Multithreading needs to use WINDOWS.
LINKEROPTS += -SUBSYSTEM:CONSOLE
# WHY IS LNK4098 HAPPENING (no ill effects from brief checks)
LINKEROPTS += -NODEFAULTLIB:msvcrt.lib

# List of compiled .obj files corresponding to STUDENT_LIBS,
# e.g. "out/vector.obj".
# Don't worry about the syntax; it's just adding "out/" to the start
# and ".obj" to the end of each value in STUDENT_LIBS.
STUDENT_OBJS = $(addprefix out/,$(STUDENT_LIBS:=.obj))
# List of test suite executables, e.g. "bin/test_suite_vector.exe"
TEST_BINS = $(addsuffix .exe,$(addprefix bin/test_suite_,$(STUDENT_LIBS)))
# List of demo executables, i.e. "bin/bounce.exe".
DEMO_BINS = $(addsuffix .exe,$(addprefix bin/,$(DEMOS)))
# All executables (the concatenation of TEST_BINS and DEMO_BINS)
BINS = $(TEST_BINS) $(DEMO_BINS)

# The first Make rule. It is relatively simple:
# "To build 'all', make sure all files in BINS are up to date."
# You can execute this rule by running the command "make all", or just "make".
all: $(BINS)

# Any .o file in "out" is built from the corresponding C file.
# Although .c files can be directly compiled into an executable, first building
# .o files reduces the amount of work needed to rebuild the executable.
# For example, if only list.c was modified since the last build, only list.o
# gets recompiled, and clang reuses the other .o files to build the executable.
#
# "%" means "any string".
# Unlike "all", this target has a build command.
# "$^" is a special variable meaning "the source files"
# and $@ means "the target file", so the command tells clang
# to compile the source C file into the target .obj file. (via -Fo)
out/%.obj: library/%.c # source file may be found in "library"
	$(CC) -c $^ $(CFLAGS) -Fo"$@"
out/%.obj: demo/%.c # or "demo"
	$(CC) -c $^ $(CFLAGS) -Fo"$@"
out/%.obj: tests/%.c # or "tests"
	$(CC) -c $^ $(CFLAGS) -Fo"$@"

bin/bounce.exe bin\bounce.exe: out/bounce.obj out/sdl_wrapper.obj $(STUDENT_OBJS)
	$(CC) $^ $(CFLAGS) -link $(LINKEROPTS) $(LIBS) -out:"$@"

bin/gravity.exe bin\gravity.exe: out/gravity.obj out/sdl_wrapper.obj $(STUDENT_OBJS)
	$(CC) $^ $(CFLAGS) -link $(LINKEROPTS) $(LIBS) -out:"$@"

bin/pacman.exe bin\pacman.exe: out/pacman.obj out/sdl_wrapper.obj $(STUDENT_OBJS)
	$(CC) $^ $(CFLAGS) -link $(LINKEROPTS) $(LIBS) -out:"$@"

bin/nbodies.exe: out/nbodies.obj out/sdl_wrapper.obj $(STUDENT_OBJS)
	$(CC) $^ $(CFLAGS) -link $(LINKEROPTS) $(LIBS) -out:"$@"

bin/damping.exe: out/damping.obj out/sdl_wrapper.obj $(STUDENT_OBJS)
	$(CC) $^ $(CFLAGS) -link $(LINKEROPTS) $(LIBS) -out:"$@"

bin/spaceinvaders.exe: out/spaceinvaders.obj out/sdl_wrapper.obj $(STUDENT_OBJS)
	$(CC) $^ $(CFLAGS) -link $(LINKEROPTS) $(LIBS) -out:"$@"

# Builds the test suite executables from the corresponding test .o file
# and the library .o files. The only difference from the demo build command
# is that it doesn't link the SDL libraries.
bin/test_suite_%.exe bin\test_suite_%.exe: out/test_suite_%.obj out/test_util.obj $(STUDENT_OBJS)
	$(CC) $^ $(CFLAGS) -link $(LINKEROPTS) -out:"$@"

# Empty recipes for cross-OS task compatibility.
bin/bounce bin\bounce: bin/bounce.exe ;
bin/gravity bin\gravity: bin/gravity.exe ;
bin/pacman bin\pacman: bin/pacman.exe ;
bin/nbodies bin\nbodies: bin/nbodies.exe; 
bin/damping bin\damping: bin/damping.exe; 
bin/spaceinvaders bin\spaceinvaders: bin/spaceinvaders.exe
bin/test_suite_% bin\test_suite_%: bin/test_suite_%.exe ;

# CMD commands to test and clean

bin/student_tests.exe: out/student_tests.obj out/test_util.obj $(STUDENT_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

# "$(subst /,\, $(TEST_BINS))" replaces "/" with "\" for
#	Windows paths,
# "echo %%i.exe" prints the test suite,
# "cmd /c %%i.exe" runs the test,
# "|| exit /b" causes the session to exit if any of the tests fail,
# "echo." prints a newline.
test: $(TEST_BINS)
	for %%i in ($(subst /,\, $(TEST_BINS))) \
	do ((echo %%i) && ((cmd /c %%i) || exit /b) && (echo.))

# Explicitly iterate on files in out\* and bin\*, and
# delete if it's not .gitignore
clean:
	for %%i in (out\* bin\*) \
	do (if not "%%~xi" == ".gitignore" del %%~i)

# This special rule tells Make that "all", "clean", and "test" are rules
# that don't build a file.
.PHONY: all clean test
# Tells Make not to delete the .obj files after the executable is built
.PRECIOUS: out/%.obj

endif
