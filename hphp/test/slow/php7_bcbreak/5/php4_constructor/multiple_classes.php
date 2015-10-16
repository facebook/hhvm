<?php

// You can have multiple deprecation warnings for PHP 4 style constructors

class Filter {
  // HHVM (PHP 5 mode), PHP 4, 5: filter is a constructor
  // HHVM (PHP 7 mode), PHP 7: filter is a constructor and
  //                           E_DEPRECATED is raised
  // PHP 8: filter is a normal method and is not a constructor;
  //        no E_DEPRECATED is raised
  function filter() {
    echo "In Filter PHP 4 style constructor\n";
  }
}

class Mapper {
  // HHVM (PHP 5 mode), PHP 4, 5: filter is a constructor
  // HHVM (PHP 7 mode), PHP 7: filter is a constructor and
  //                           E_DEPRECATED is raised
  // PHP 8: filter is a normal method and is not a constructor;
  //        no E_DEPRECATED is raised
  function mapper() {
    echo "In Mapper PHP 4 style constructor\n";
  }
}

$f = new Filter(); // calls the PHP 4 style constructor
$f->filter(); // calls the method filter, which is also the PHP4 style ctor
$m = new Mapper();  // calls the PHP 4 style constructor
$m->mapper(); // calls the method mapper, which is also the PHP4 style ctor
