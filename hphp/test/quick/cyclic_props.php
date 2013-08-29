<?php
class x {
  public $x0;
  public $y1;
}

function cyclic_prop_declared_props() {
  $a = new x;
  $a->x0 = new x;
  $a->x0->y1 =& $a->x0;
  var_dump($a);
  var_dump($a->x0->y1 = "ok");
}

function cyclic_prop_nondeclared_props() {
  $a = new stdClass;
  $a->x0 = new stdClass;
  $a->x0->y0 = 'a';
  $a->x0->y1 =& $a->x0;
  $a->x0->y2 =& $a->x0;
  $a->x0->y0 = 'b';
  var_dump($a);
  var_dump($a->x0->y1 = "ok");
  var_dump($a->x0);
}

cyclic_prop_declared_props();
cyclic_prop_nondeclared_props();
