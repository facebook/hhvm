<?hh

function test($x) {
  $f = $x ==> {
    return $x is FooBarBaz;
  };

  return $f($x);
}
<<__EntryPoint>> function main(): void {
var_dump(test(new stdClass));
}
