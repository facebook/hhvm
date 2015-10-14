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

class FilterX extends Filter {
  function __construct() {
    // HHVM (PHP 5 mode, PHP 7 mode), PHP 5, 7: Filter::filter is called;
    //                                          no error
    // PHP 8: "Fatal error: Cannot call constructor"
    parent::__construct();
  }

}

new FilterX();
