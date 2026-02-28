<?hh

<<file: __EnableUnstableFeatures('shape_destructure')>>
;

class C {
  const string d = 'a';
}

<<__EntryPoint>>
function main(): void {
  $shape = dict[
    'k1' => shape('a' => 1, 'b' => 2, 'c' => 3),
    'k2' => shape('a' => 4, 'b' => 5, 'c' => 6),
  ];
  foreach ($shape as $k => shape('c' => $c, 'a' => $a, 'b' => $b)) {
    var_dump($k);
    var_dump($a);
    var_dump($b);
    var_dump($c);
  }

  $shape2 = dict[
    'k1' => shape('a' => 1, 'b' => 2, nameof C => 3),
    'k2' => shape('a' => 4, 'b' => 5, nameof C => 6),
  ];
  foreach ($shape2 as $k2 => shape(nameof C => $c2, 'a' => $a2, 'b' => $b2)) {
    var_dump($k2);
    var_dump($a2);
    var_dump($b2);
    var_dump($c2);
  }

  $shape3 = dict['k1' => shape('C' => 10), 'k2' => shape('C' => 100)];
  foreach ($shape3 as $k3 => shape(nameof C => $a3)) {
    var_dump($k3);
    var_dump($a3);
  }

  $shape4 = vec[shape(nameof C => 11), shape(nameof C => 110)];
  foreach ($shape4 as shape('C' => $a4)) {
    var_dump($a4);
  }

  $shape5 = vec[shape(C::d => 12), shape('a' => 120)];
  foreach ($shape5 as shape(C::d => $d)) {
    var_dump($d);
  }
}
