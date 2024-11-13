<?hh

<<file: __EnableUnstableFeatures('shape_destructure')>>
;

class C {
  const string d = 'a';
}

<<__EntryPoint>>
function main(): void {
  $v = vec[0];
  tuple(
    $a,
    shape(
      'b' => $b,
      'v0' => $v[0],
      'l' => tuple($c, $d),
      's' => shape('nest' => $nest, C::d => $e),
    ),
  ) = tuple(1, shape(
    'b' => 2,
    'v0' => 3,
    'l' => tuple(4, 5),
    's' => shape('nest' => 6, C::d => 7),
  ));

  var_dump($a);
  var_dump($b);
  var_dump($v[0]);
  var_dump($c);
  var_dump($d);
  var_dump($nest);
  var_dump($e);
}
