<?php

// Namespaced classes do not recognize PHP 4 constructors
namespace PHP4CtorNS;

class Filter {
    // HHVM (PHP 5 mode, PHP 7 mode), PHP 4, 5, 7, 8: filter is a constructor
  function filter($a) {
    echo "In Filter::filter method\n";
  }
}

$f = new Filter();
$f->filter(3);
