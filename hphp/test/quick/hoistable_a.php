<?hh

print "Test begin\n";

if (!isset($g)) {
  include 'hoistable_a.inc';
}

class B extends A {}

print "Test end\n";
