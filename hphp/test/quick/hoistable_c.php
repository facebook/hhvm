<?php

# Make the verify script happy.
print "1..4\n";
if (!class_exists('B')) print "ok 1\n";
class B extends A {}
class A {}

interface I {}
class X implements I {}
if (!class_exists('Y')) print "ok 2\n";
class Y extends X {}

if (!isset($g)) {
  class U {}
}
if (!class_exists('V')) print "ok 3\n";
class V extends U {}

trait T {}
if (!class_exists('S')) print "ok 4\n";
class S {
  use T;
}
