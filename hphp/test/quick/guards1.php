<?php

// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

class C {}

function foo($x, $y) {
   echo "$x $y\n";
}

foo(1, 1);
foo(1, 2.1);
foo(1, true);
foo(1, array(1));

foo(2.1, 1);
foo(2.1, 2.1);
foo(2.1, true);
foo(2.1, array(1));

foo(true, 1);
foo(true, 2.1);
foo(true, true);
foo(true, array(1));

foo(array(1), 1);
foo(array(1), 2.1);
foo(array(1), true);
foo(array(1), array(1));

/*  $arr = array(1 => 2, 2 => true, 3 => $uninit, 4 => "string", 5 => array(1), 6 => 6.6,
               "1" => 2, "2" => true, "3" => $uninit, "4" => "string", "5" => array(1), "6" => 6.6);

  foreach ($arr as $k => $v) {
    echo "$k => $v\n";
  }
}

foo();
*/
