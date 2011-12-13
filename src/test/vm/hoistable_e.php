<?php

# Make the verify script happy.
print "1..1\n";
print "ok 1\n";

if (isset($g)) {
  class X {}
}
var_dump(new Y);
class Y extends X {}
