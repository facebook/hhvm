<?hh

namespace WhateverNamespace;

class WhateverTest {
  public function filter( $array ) :mixed{
    $mapFunction = function ( $item ) {
      return "whatever";
    };

    return \array_map( $mapFunction, $array );
  }
}


<<__EntryPoint>>
function main_systemlibhhas() :mixed{
$t = new WhateverTest();
$ret = $t->filter( vec[  "lol", "wow", "doge" ] );

\var_dump( $ret );
}
