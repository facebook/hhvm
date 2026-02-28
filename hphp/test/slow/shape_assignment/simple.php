<?hh

<<file:__EnableUnstableFeatures('shape_destructure')>>;

class C {
  const string d = 'a';
}

<<__EntryPoint>>
function main(): void {
  $shape = shape('a' => 1, 'b' => 2, 'c' => 3);
  shape('c' => $c, 'a' => $a, 'b' => $b) = $shape;
  var_dump($a);
  var_dump($b);
  var_dump($c);


  $shape2 = shape('a' => 1, 'b' => 2, nameof C => 3);
  shape(nameof C => $c2, 'a' => $a2, 'b' => $b2) = $shape2;
  var_dump($a2);
  var_dump($b2);
  var_dump($c2);

  $shape3 = shape('C' => 10);
  shape(nameof C => $a3) = $shape3;
  var_dump($a3);

  $shape4 = shape(nameof C => 11);
  shape('C' => $a4) = $shape4;
  var_dump($a4);

  $shape5 = shape(C::d => 12);
  shape(C::d => $d) = $shape5;
  var_dump($d);

  $shape6 = shape(C::d => 13);
  shape('a' => $d2) = $shape6;
  var_dump($d2);

  $shape7 = shape('a' => 14);
  shape(C::d => $d3) = $shape7;
  var_dump($d3);
}
