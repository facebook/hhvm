<?hh

<<__EntryPoint>> function main() :mixed{
  include "prop-ub-class-1.inc";
  include "prop-type-alias.inc";
  include "prop-ub-class-2.inc";

  $o = new Foo;
  $o->x = 3.14;
  $o->x = vec[1];
  $o->y = null;
  $o->y = new Bar;
  $o->y = new NoBar;
  Foo::$sx = null;
  Foo::$sx = 'd';
  Foo::$sy = new Bar;;
  Foo::$sy = vec[2];
  $o->z = 10;
  $o->z = vec[3];
  $o->w = null;
  $o->v = null; // not enforced
  $o->u = 'a';  // not enforced
}
