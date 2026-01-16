<?hh



function test_variadic(string $s, int... $args):void {
  var_dump($s);
  var_dump($args);
}

function test_splat(string $s, ...(int,bool) $args):void {
  var_dump($s);
  var_dump($args);
}

function test_poly_splat<T as (mixed...)>(string $s, ...T $args):void {
  var_dump($s);
  var_dump($args);
}

<<__EntryPoint>>
function main():void {
  $t = tuple(3, true);
  test_variadic("A", 1);
  test_variadic("B", 1, 2, 3);
  test_splat("C", 2, false);
  test_splat("D", ...$t);
  test_poly_splat("D", 1.2, 3);
  test_poly_splat("D", ...$t);
}
