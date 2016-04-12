# Due to HHVM's size, we need to use the native 64-bit toolchain to get a decent link time,
# and, in fact, it's also needed to be able to link at all in debug mode, due to the size of
# the hphp_runtime_static static library. However, Visual Studio defaults to using the 32-bit
# hosted cross compiler targetting 64-bit. Unfortunately, CMake doesn't provide us a way to
# do this, so we have to resort to a hack-around in order to make this possible. Because the
# toolset value is put into the project file unescaped, we can use it to add the PreferredToolArchitecture
# value that we need, as long as we make sure to properly close and re-open the current tags.
#
# To add support for newer MSVC versions, simply adjust the actual toolset value at the start
# and end of the string.
#
# Unfortunately, we can't rely on the MSVC and MSVC14 variables to check if we need to enable
# this, due to the fact they are set when the C/CXX languages are enabled, however, this value
# needs to be set before the languages are enabled in order to have any effect, so we set it
# based directly off of the name of the generator, which is set before configuration even begins.

if ("${CMAKE_GENERATOR}" STREQUAL "Visual Studio 14 2015 Win64")
    set(CMAKE_GENERATOR_TOOLSET "v140</PlatformToolset><PreferredToolArchitecture>x64</PreferredToolArchitecture><PlatformToolset>v140")
endif()
