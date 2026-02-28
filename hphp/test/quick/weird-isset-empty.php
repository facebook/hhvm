<?hh
function tst1(inout $abc) :mixed{
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
function tst2(inout $abc) :mixed{
  $abc->foo = dict['bar' => 'baz'];
  var_dump(isset($abc->foo));
  var_dump(!($abc->foo ?? false));
  var_dump(isset($abc->foo['bar']));
  var_dump(!($abc->foo['bar'] ?? false));
  var_dump(isset($abc->foo->bar));
  var_dump(!($abc->foo->bar ?? false));
}
function tst3(inout $abc) :mixed{
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

  $x = vec["a","b","c"];
  $y = 0;
  var_dump(isset($x[$y]));
  var_dump(!($x[$y] ?? false));
  var_dump(isset($x->$y));
  var_dump(!($x->$y ?? false));
  var_dump($x);

  $x = vec["a","b","c"];
  $y = "0";
  var_dump(isset($x[$y]));
  var_dump(!($x[$y] ?? false));
  var_dump(isset($x->$y));
  var_dump(!($x->$y ?? false));
  var_dump($x);

  echo "**************************\n";

  $x = vec[null];
  $y = 0;
  var_dump(isset($x[$y]));
  var_dump(!($x[$y] ?? false));
  var_dump(isset($x->$y));
  var_dump(!($x->$y ?? false));
  var_dump($x);

  $x = vec[null];
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

  $abc = dict['foo' => dict['bar' => 'baz']];
  tst1(inout $abc);  // make $abc a Var

  echo "**************************\n";

  $abc = new stdClass;
  $abc->foo = dict['bar' => 'baz'];
  var_dump(isset($abc->foo));
  var_dump(!($abc->foo ?? false));
  var_dump(isset($abc->foo['bar']));
  var_dump(!($abc->foo['bar'] ?? false));
  var_dump(isset($abc->foo->bar));
  var_dump(!($abc->foo->bar ?? false));
  unset($abc);

  echo "**************************\n";

  $abc = new stdClass;
  tst2(inout $abc);  // make $abc a Var
  unset($abc);

  echo "**************************\n";

  $abc = dict['foo' => new stdClass];
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

  $abc = dict['foo' => new stdClass];

  tst3(inout $abc);  // make $abc a Var
  unset($abc);

  echo "**************************\n";
}
