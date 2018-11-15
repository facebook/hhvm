<?hh

function test($x) {
  $f = $x ==> {
    return $x is FooBarBaz;
  };

  return $f($x);
}

var_dump(test(new stdclass));
