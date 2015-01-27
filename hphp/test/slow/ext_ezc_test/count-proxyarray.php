<?php

// Ensure that count() and conversion to boolean work with proxy arrays

$x = ezc_create_cloneable_in_array();
print count($x) . "\n";
ezc_call(
  function( $x ) {
    print $x ? "true\n" : "false\n";
  },
  array()
);
