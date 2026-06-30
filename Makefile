####################################################
# C/C++
####################################################
# Target file
target := school_vulkan

# CPP compiler
compiler := g++

# CPP version
cpp_ver := c++20

# Directories
target_dir  := build
src_dir     := src
include_dir := include
lib_dir     := lib
obj_dir     := objs

# Directory contains resources for program
assets_dir := assets

# If you need to connect to external include and library directories,
# note that these directories won't be created when using "make init".
# Note: When assigning multiple values to the same variable in a Makefile,
# remember to add a backslash (\) at the end of each line.
ext_include_dir := 
ext_lib_dir     := 

# Compiler flags
compile_flags := -g -Wall -Wextra -MMD -MP

# Define Macros
macros := NDEBUG

# Linker flags
linker_flags := -static
# Link libraries, this will be append -l prefix automatically. For example, "gdi32" will become -lgdi32.
# So just type the libraries name you want to link, like "gdi32 opengl32 pthread".
libraries := vulkan-1 glfw3 gdi32


# Source file suffix configurations (Not recommended to change)
src_suffix := .cpp .cxx .cp .c++ .CPP .cc .c .C



####################################################
# SPIR-V
####################################################
# Shaders compiler
spv_compiler := tools/glslc

# Shaders Directories
spv_shaders_src_dir    := $(src_dir)/shaders
spv_shaders_target_dir := $(target_dir)/shaders


# Source file suffix configurations (Not recommended to change)
spv_shaders_src_suffix := .vert .frag .geom .tesc .tese .rgen .rahit .rchit .rmiss .rcall


####################################################
####################################################
include tacomake/platform.mk
include tacomake/functions.mk
include tacomake/rules.mk
include tacomake/commands.mk


####################################################
# Project-specific SPIR-V shader build
####################################################
override spv_shaders_srcs := $(sort $(foreach d,$(spv_shaders_src_suffix),$(call rwildcard,$(spv_shaders_src_dir),*$d)))
override spv_shaders_targets := $(patsubst $(spv_shaders_src_dir)/%,$(spv_shaders_target_dir)/%.spv,$(spv_shaders_srcs))
override spv_mirror_dirs := $(call extrdir,$(spv_shaders_targets))

$(build_target): $(spv_shaders_targets)

$(spv_shaders_target_dir)/%.spv: $(spv_shaders_src_dir)/% | $(spv_mirror_dirs)
	$(call msg,Compiling GLSL shader file "$<" to SPIR-V shader "$@".)
	@$(call fixpath,$(spv_compiler)) $(call fixpath,$<) -o $(call fixpath,$@)
	$(call msg,Compiling finished!)

$(spv_mirror_dirs):
	$(call msg,Deteced missing directory "$@"$(comma) create new one...)
	@$(call mkdir,$(call fixpath,$@))
