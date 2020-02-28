<?hh

namespace WhateverNamespace;

class WhateverTest {
  public function filter( $array ) {
    $mapFunction = function ( $item ) {
      return "whatever";
    };

    return \array_map( $mapFunction, $array );
  }
}


<<__EntryPoint>>
function main_systemlibhhas() {
$t = new WhateverTest();
$ret = $t->filter( varray[  "lol", "wow", "doge" ] );

\var_dump( $ret );
}
