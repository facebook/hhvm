<?php
// Defining __construct and filter makes filter a normal method
class Filter {
  // PHP 4/5: E_STRICT "Redefining already defined constructor"
  // HHVM (PHP 5 mode), HHVM (PHP 7 mode), PHP 7, 8: No error is raised
  function filter($a) {
    echo "In Filter::filter method\n";
  }

  // This raises the E_STRICT in PHP 4/5, but not in HHVM or PHP 7+
  function __construct() {
    echo "In Filter constructor\n";
  }
}

$f = new Filter();
$f->filter(3);
