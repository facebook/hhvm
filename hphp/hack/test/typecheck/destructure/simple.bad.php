<?hh
<<file:__EnableUnstableFeatures('shape_destructure')>>

function f(): void {
  tuple($a, $b) = tuple(1, 2, 3);
  tuple($a, $b) = vec[1,2,3];
  $v = Vector<int>{1, 2};
  tuple($v[0], $a) = tuple("a", 2);

  shape('a' => $a, 'b' => $b) = shape('b' => 2);
  shape('b' => $b) = shape('a' => 1, 'b' => 2);


}
