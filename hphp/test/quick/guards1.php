<?hh

class C {}

function foo($x, $y) {
  $x = HH\is_any_array($x) ? 'Array' : $x;
  $y = HH\is_any_array($y) ? 'Array' : $y;
  echo "$x $y\n";
}

<<__EntryPoint>> function main(): void {

foo(1, 1);
foo(1, 2.1);
foo(1, true);
foo(1, varray[1]);

foo(2.1, 1);
foo(2.1, 2.1);
foo(2.1, true);
foo(2.1, varray[1]);

foo(true, 1);
foo(true, 2.1);
foo(true, true);
foo(true, varray[1]);

foo(varray[1], 1);
foo(varray[1], 2.1);
foo(varray[1], true);
foo(varray[1], varray[1]);

/*
  $arr = array(1 => 2, 2 => true, 3 => $uninit, 4 => "string", 5 => array(1), 6 => 6.6,
               "1" => 2, "2" => true, "3" => $uninit, "4" => "string", "5" => array(1), "6" => 6.6);
  foreach ($arr as $k => $v) {
    echo "$k => $v\n";
  }
*/
}
