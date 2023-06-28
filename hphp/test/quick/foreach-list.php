<?hh

function test_simple() :mixed{
  $arr = varray[
    varray[1,2],
    null,
    varray[3,4]
  ];
  foreach($arr as list($a, $b)) {
    var_dump($a, $b);
  }
}

function test_nested() :mixed{
  $arr = varray[
    varray[1, varray[2,3], 4],
    varray[5, varray[6,7], 8],
    null,
    varray[9, varray[10, 11], 12]
  ];

  foreach ($arr as list($a, list($b, $c), $d)) {
    var_dump($d, $c, $b, $a);
  }
}

function test_single() :mixed{
  $arr = varray[
    varray[1], varray[2]
  ];
  foreach($arr as list($a)) {
    var_dump($a);
  }
}

function gen() :AsyncGenerator<mixed,mixed,void>{
  yield varray[1,2] => 3;
  yield varray[4,5] => 6;
}
function test_key() :mixed{
  foreach (gen() as list($a, $b) => $c) {
    var_dump($c, $a, $b);
  }
}

function gen2() :AsyncGenerator<mixed,mixed,void>{
  yield varray[1,varray[2,3],4] => varray[varray[1,2],varray[3,4]];
  yield varray[1,null,2] => varray[null, varray[1,2]];
  yield null => null;
  yield varray[1,varray[2,3,4],5] => varray[varray[1,2],varray[3,4],varray[5,6]];
}
function test_complex() :mixed{
  foreach (gen2() as
           list($a, list($b, $c), $d) =>
           list(list($e, $f), list($g, $h))) {
    var_dump($b, $a, $d, $c, $f, $e, $h, $g);
  }
}
<<__EntryPoint>> function main(): void {
test_simple();
test_nested();
test_single();
test_key();
test_complex();
}
