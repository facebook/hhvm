<?hh

class C {}

function foo($x, $y) :mixed{
  $x = HH\is_any_array($x) ? 'Array' : $x;
  $y = HH\is_any_array($y) ? 'Array' : $y;
  $x__str = (string)($x);
  $y__str = (string)($y);
  echo "$x__str $y__str\n";
}

<<__EntryPoint>> function main(): void {

foo(1, 1);
foo(1, 2.1);
foo(1, true);
foo(1, vec[1]);

foo(2.1, 1);
foo(2.1, 2.1);
foo(2.1, true);
foo(2.1, vec[1]);

foo(true, 1);
foo(true, 2.1);
foo(true, true);
foo(true, vec[1]);

foo(vec[1], 1);
foo(vec[1], 2.1);
foo(vec[1], true);
foo(vec[1], vec[1]);

/*
  $arr = array(1 => 2, 2 => true, 3 => $uninit, 4 => "string", 5 => array(1), 6 => 6.6,
               "1" => 2, "2" => true, "3" => $uninit, "4" => "string", "5" => array(1), "6" => 6.6);
  foreach ($arr as $k => $v) {
    echo "$k => $v\n";
  }
*/
}
