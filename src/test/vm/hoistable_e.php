<?php

if (!isset($g)) {
  # Make the verify script happy.
  print "1..1\n";
  class X {}
  if (!class_exists('Y')) print "ok 1\n";
} else {
  var_dump(new Y);
}
class Y extends X {}
