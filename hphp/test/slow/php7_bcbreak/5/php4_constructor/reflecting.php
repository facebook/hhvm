<?php

class Filter {
  // HHVM (PHP 5 mode), PHP 5: filter is a constructor
  // HHVM (PHP 7 mode), PHP 7: filter is a constructor and E_DEPRECATED is
  //                           raised
  // PHP 8: filter is a normal method and is not a constructor;
  //        no E_DEPRECATED is raised
  function filter() {
    echo "In Filter PHP 4 style constructor\n";
  }
}

$f = new Filter(); // calls the PHP 4 style constructor
$f->filter(); // calls the method filter, which is also the PHP4 style ctor

$refMethFilter = new ReflectionMethod('Filter', 'filter');
// HHVM (PHP 5 mode, PHP 7 mode), PHP 5, 7: bool(true)
// PHP 8: bool(false)
var_dump($refMethFilter->isConstructor());
