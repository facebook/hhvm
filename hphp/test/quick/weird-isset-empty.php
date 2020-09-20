<?hh
function tst1(inout $abc) {
  var_dump(isset($abc['foo']));
  var_dump(!($abc['foo'] ?? false));
  var_dump(isset($abc->foo));
  var_dump(!($abc->foo ?? false));
  var_dump(isset($abc['foo']['bar']));
  var_dump(!($abc['foo']['bar'] ?? false));
  var_dump(isset($abc['foo']->bar));
  var_dump(!($abc['foo']->bar ?? false));
  var_dump(isset($abc->foo['bar']));
  var_dump(!($abc->foo['bar'] ?? false));
  var_dump(isset($abc->foo->bar));
  var_dump(!($abc->foo->bar ?? false));
}
function tst2(inout $abc) {
  $abc->foo = darray['bar' => 'baz'];
  var_dump(isset($abc->foo));
  var_dump(!($abc->foo ?? false));
  var_dump(isset($abc->foo['bar']));
  var_dump(!($abc->foo['bar'] ?? false));
  var_dump(isset($abc->foo->bar));
  var_dump(!($abc->foo->bar ?? false));
}
function tst3(inout $abc) {
  $abc['foo']->bar = 'baz';
  var_dump(isset($abc['foo']));
  var_dump(!($abc['foo'] ?? false));
  var_dump(isset($abc->foo));
  var_dump(!($abc->foo ?? false));
  var_dump(isset($abc['foo']->bar));
  var_dump(!($abc['foo']->bar ?? false));
  var_dump(isset($abc->foo['bar']));
  var_dump(!($abc->foo['bar'] ?? false));
  var_dump(isset($abc->foo->bar));
  var_dump(!($abc->foo->bar ?? false));
}
<<__EntryPoint>>
function main_entry(): void {

  $x = varray["a","b","c"];
  $y = 0;
  var_dump(isset($x[$y]));
  var_dump(!($x[$y] ?? false));
  var_dump(isset($x->$y));
  var_dump(!($x->$y ?? false));
  var_dump($x);

  $x = varray["a","b","c"];
  $y = "0";
  var_dump(isset($x[$y]));
  var_dump(!($x[$y] ?? false));
  var_dump(isset($x->$y));
  var_dump(!($x->$y ?? false));
  var_dump($x);

  echo "**************************\n";

  $x = varray[null];
  $y = 0;
  var_dump(isset($x[$y]));
  var_dump(!($x[$y] ?? false));
  var_dump(isset($x->$y));
  var_dump(!($x->$y ?? false));
  var_dump($x);

  $x = varray[null];
  $y = "0";
  var_dump(isset($x[$y]));
  var_dump(!($x[$y] ?? false));
  var_dump(isset($x->$y));
  var_dump(!($x->$y ?? false));
  var_dump($x);

  echo "**************************\n";

  $x = "abc";
  $y = 0;
  var_dump(isset($x[$y]));
  var_dump(!($x[$y] ?? false));
  var_dump(isset($x->$y));
  var_dump(!($x->$y ?? false));
  var_dump($x);

  $x = "abc";
  $y = "0";
  var_dump(isset($x[$y]));
  var_dump(!($x[$y] ?? false));
  var_dump(isset($x->$y));
  var_dump(!($x->$y ?? false));
  var_dump($x);

  echo "**************************\n";

  $abc = darray['foo' => darray['bar' => 'baz']];
  tst1(inout $abc);  // make $abc a Var

  echo "**************************\n";

  $abc = new stdclass;
  $abc->foo = darray['bar' => 'baz'];
  var_dump(isset($abc->foo));
  var_dump(!($abc->foo ?? false));
  var_dump(isset($abc->foo['bar']));
  var_dump(!($abc->foo['bar'] ?? false));
  var_dump(isset($abc->foo->bar));
  var_dump(!($abc->foo->bar ?? false));
  unset($abc);

  echo "**************************\n";

  $abc = new stdclass;
  tst2(inout $abc);  // make $abc a Var
  unset($abc);

  echo "**************************\n";

  $abc = darray['foo' => new stdclass];
  $abc['foo']->bar = 'baz';
  var_dump(isset($abc['foo']));
  var_dump(!($abc['foo'] ?? false));
  var_dump(isset($abc->foo));
  var_dump(!($abc->foo ?? false));
  var_dump(isset($abc['foo']->bar));
  var_dump(!($abc['foo']->bar ?? false));
  var_dump(isset($abc->foo['bar']));
  var_dump(!($abc->foo['bar'] ?? false));
  var_dump(isset($abc->foo->bar));
  var_dump(!($abc->foo->bar ?? false));
  unset($abc);

  echo "**************************\n";

  $abc = darray['foo' => new stdclass];

  tst3(inout $abc);  // make $abc a Var
  unset($abc);

  echo "**************************\n";
}
