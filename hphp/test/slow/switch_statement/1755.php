<?php

class Evil {
  private $x;
  public function __construct() {
 $this->x = 0;
 }
  public function __toString() {
    return sprintf("Evil%d", $this->x++);
  }
}
function f_1($x) {
  switch ($x) {
  case "123": print '"123"' . "";
 break;
  case "4abc": print '"4abc"' . "";
 break;
  default: print "default";
 break;
  case "0": print '"0"' . "";
 break;
  case "": print '""' . "";
 break;

  case "Evil4": print '"Evil4"' . "";
 break;
  }
}
function f_2($x) {
  var_dump($x);
 print "  goes to:";
  switch ($x) {
  case "foo": print "foo";
 break;
  case "1": print "1";
 break;
  case "2.0": print "2.0";
 break;
  case "2ab": print "2ab";
 break;
  case "3.212": print "3.212";
 break;
  case "0": print "0";
 break;
  case "": print "{empty str}";
 break;
  default: print "default";
 break;
  }
}
function g_2($x) {
  var_dump($x);
 print "  goes to:";
  switch ($x) {
  case "": print "{empty str}";
 break;
  case "0": print "0";
 break;
  default: print "default";
 break;
  }
}
function h_2($x) {
  var_dump($x);
 print "  goes to:";
  switch ($x) {
  case "3.0": print "3.0";
 break;
  case "3.0abc": print "3.0abc";
 break;
  case "3": print "3";
 break;
  default: print "";
  }
}
function f_3($x) {
  switch ($x) {
  default: print "default";
  case "bar": print "bar";
  case "foo": print "foo";
  case "baz": print "baz";
  }
}
f_1("");
f_1(null);
f_1(false);
f_1("0");
f_1("0eab");
f_1("0.0");
f_1(0.0);
f_1(0);
f_1(true);
f_1(false);
f_1("4abc");
f_1(4);
f_1("4.0");
f_1(new Evil());
f_2(1);
f_2(2);
f_2(2.0);
f_2(true);
f_2(false);
f_2(null);
f_2((object) null);
f_2(array());
f_2(3.21200);
g_2(0);
g_2(null);
g_2(false);
g_2(true);
h_2("3");
h_2("3abc");
h_2("3a");
h_2(3);
h_2(3.0);
f_3("foo");
f_3("bar");
f_3("baz");
f_3("def");
