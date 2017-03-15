<?php

namespace WhateverNamespace;

class WhateverTest {
  public function filter( $array ) {
    $mapFunction = function ( $item ) {
      return "whatever";
    };

    return array_map( $mapFunction, $array );
  }
}

$t = new WhateverTest();
$ret = $t->filter( array(  "lol", "wow", "doge" ) );

var_dump( $ret );
