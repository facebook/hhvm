<?php

class Blah {
  public $x = "uncollected string";
}

class Cycler {
  public function __destruct() { echo "Shouldn't run\n"; }

  public $x = null;
  public $y;
}

function test_cycles($what) {
  echo "-------------------" . $what . "---------------------------\n";
  $file = tempnam("/tmp", "cycles_unit_test");
  fb_gc_detect_cycles($file);
  echo @file_get_contents($file);
  @unlink($file);
  echo fb_gc_collect_cycles();
}

function decl_prop_cycle() {
  global $x;
  $x = new Blah();

  // y <---> x
  $y = new Cycler();
  $y->y = "this is a string";
  $y->x = new Cycler();
  $y->x->x = $y;

  /*
   * y2 ---> y3 -----> array(Cycler())
   * ^        |
   * |        |
   * +-- y4 <-+
   */
  $y2 = new Cycler();
  $y3 = new Cycler();
  $y4 = new Cycler();
  $y2->x = $y3;
  $y3->x = $y4;
  $y4->x = $y2;
  $y3->y = array(new Cycler());
}

decl_prop_cycle();
test_cycles("decl");
var_dump($x->x);

function dyn_prop_cycle() {
  $y = new Cycler();
  $y->d_y = "this is a string";
  $y->d_x = new Cycler();
  $y->d_x->d_x = $y;

  // Same graph as decl_prop_cycle, but will have the dynprop array in
  // between everything.
  $y2 = new Cycler();
  $y3 = new Cycler();
  $y4 = new Cycler();
  $y2->d_x = $y3;
  $y3->d_x = $y4;
  $y4->d_x = $y2;
  $y3->d_y = array(new Cycler());
}

dyn_prop_cycle();
test_cycles("dyn");
var_dump($x->x);

function array_cycle() {
  /*
   * array() ------------------> $x
   *   ^                          |
   *   |                          |
   *   |                          v
   * array() ---> array() ---> RefData
   *   ^                          |
   *   |                          |
   *   +--------------------------+
   */
  $y = array();
  $x = new Cycler();
  $x->x =& $y;
  $y["one"]["bar"] = $x;
  $y["two"]["bar"] =& $y;
  $y["three"] = "string data";
}

array_cycle();
test_cycles("array");
var_dump($x->x);
