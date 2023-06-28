<?hh

<<__EntryPoint>> function main() :mixed{
  include "prop-ub-class-1.inc";
  include "prop-type-alias.inc";
  include "prop-ub-class-2.inc";

  $o = new Foo;
  $o->x = vec[1];
  $o->y = new NoBar;
  Foo::$sx = 'd';
  Foo::$sy = vec[2];
  $o->z = vec[3];
  $o->w = null;
}
