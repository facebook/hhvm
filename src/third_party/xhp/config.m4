PHP_ARG_ENABLE(xhp, xhp,
[ --enable-xhp   Enable XHP])

PHP_REQUIRE_CXX()
if test "$PHP_XHP" = "yes"; then
  XHP_SHARED_DEPENDENCIES="libxhp.a"
  PHP_ADD_LIBRARY(stdc++,, XHP_SHARED_LIBADD)
  PHP_SUBST(XHP_SHARED_LIBADD)
  PHP_NEW_EXTENSION(xhp, ext.cpp, $ext_shared)
  PHP_ADD_MAKEFILE_FRAGMENT(Makefile.frag)
fi
