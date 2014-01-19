<?php

namespace foo\bar;

#===============================================================================
# ReflectionFunction.

function &f($a, &$b, $c=null) {
  static $staticX = 4;
  static $staticY;
  print "In f()\n";
  $staticX++;
  $x = $staticX;
  return $x;
}

class Test {
  public function test() {
  }
}

$rf = new \ReflectionFunction('\foo\bar\f');
print "--- getShortName(\"\\foo\\bar\\f\") ---\n";
var_dump($rf->getShortName());
print "\n";
print "--- getNamespaceName(\"\\foo\\bar\\f\") ---\n";
var_dump($rf->getNamespaceName());
print "\n";

$rf = new \ReflectionMethod('\foo\bar\Test', 'test');
print "--- getShortName(\"\\foo\\bar\\Test::test\") ---\n";
var_dump($rf->getShortName());
print "\n";
print "--- getNamespaceName(\"\\foo\\bar\\Test::test\") ---\n";
var_dump($rf->getNamespaceName());
print "\n";

$rf = new \ReflectionFunction('\strlen');
print "--- getShortName(\"strlen\") ---\n";
var_dump($rf->getShortName());
print "\n";
print "--- getNamespaceName(\"strlen\") ---\n";
var_dump($rf->getNamespaceName());
print "\n";

