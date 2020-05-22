<?hh

include "prop-ub-class.inc";

<<__EntryPoint>> function main() {
  $o = new Foo;
  $o->x = vec[1];
  $o->y = new NoBar;
  Foo::$sx = 'd';
  Foo::$sy = vec[2];
  $o->z = vec[3];
  $o->w = null;
}
