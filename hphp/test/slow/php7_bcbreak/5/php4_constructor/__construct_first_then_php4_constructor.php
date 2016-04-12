<?php
// Defining __construct and filter makes filter a normal method
class Filter {
  function __construct() {
    echo "In Filter constructor\n";
  }

  // PHP 5.0.0 - ~5.3.2: E_STRICT "Redefining already defined constructor"
  // HHVM (PHP 5 mode, PHP 7 mode), PHP 5.3.3 - 5.6, 7, 8: No error is raised
  function filter($a) {
    echo "In Filter::filter method\n";
  }
}

$f = new Filter();
$f->filter(3);
