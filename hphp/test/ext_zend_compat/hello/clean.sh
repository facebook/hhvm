# Remove products from: hphpize; cmake .; make
rm -f CMakeCache.txt
rm -f cmake_install.cmake
rm -f CMakeLists.txt
rm -f Makefile
rm -rf CMakeFiles
rm -rf deps
rm -f hello.so
rm -f junk

#
# Remove products from: test/run
#
rm -f hello_test.php.diff
rm -f hello_test.php.out

if false ; then
  #
  # Remove products from: phpize; ./configure
  #
  rm -f acinclude.m4
  rm -f aclocal.m4
  rm -f autom4te.cache
  rm -f config.guess
  rm -f config.h
  rm -f config.h.in
  rm -f config.log
  rm -f config.nice
  rm -f config.status
  rm -f config.sub
  rm -f configure
  rm -f configure.in
  rm -f install-sh
  rm -f libtool
  rm -f ltmain.sh
  rm -f Makefile
  rm -f Makefile.fragments
  rm -f Makefile.global
  rm -f Makefile.objects
  rm -f missing
  rm -f mkinstalldirs
  rm -f run-tests.php
  rm -rf hello
  rm -rf include
  rm -rf build
  rm -rf modules
  rm -rf autom4te.cache
fi
