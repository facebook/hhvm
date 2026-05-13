<?hh

<<file: __EnableUnstableFeatures('shape_and_tuple_destructuring')>>

class C {
  const string d = 'a';
}

<<__EntryPoint>>
function main(): void {
  $s = vec[
    tuple(1, shape(
      'b' => 2,
      'v0' => 3,
      'l' => tuple(4, 5),
      's' => shape('nest' => 6, C::d => 7),
    )),
    tuple(8, shape(
      'b' => 9,
      'v0' => 10,
      'l' => tuple(11, 12),
      's' => shape('nest' => 13, C::d => 14),
    )),
  ];
  foreach (
    $s as tuple(
      $a,
      shape(
        'b' => $b,
        'v0' => $v0,
        'l' => tuple($c, $d),
        's' => shape('nest' => $nest, C::d => $e),
      ),
    )
  ) {
    var_dump($a);
    var_dump($b);
    var_dump($v0);
    var_dump($c);
    var_dump($d);
    var_dump($nest);
    var_dump($e);
  }
}
