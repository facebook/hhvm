<?php

class Hey { public static $x = "yup\n"; }

function foo() {
  $cls = "Hey";
  $boxer =& $cls;
  echo $cls::$x;
}
foo();
