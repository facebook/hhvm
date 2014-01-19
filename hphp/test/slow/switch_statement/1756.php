<?php

function f($x) {
  switch ($x) {
  default:
    print "default-0";
  case "foo":
    print "foo-0";
  case "3":
    print "3-0";
  default:
    print "default-1";
  case "3":
    print "3-1";
  case "foo":
    print "foo-1";
  default:
    print "default-2";
  case "bar":
    print "bar";
  }
}
function g($x) {
  switch ($x) {
  case 'x': print 'x';
 break;
  case '0': print '0';
 break;
  }
}
f("foo");
f("3");
f("bar");
f(null);
f(3);
f(0);
f(0.0);
f(3.0);
f(true);
f(false);
f(array());
f(new stdClass());
g(0);
g(0.0);
