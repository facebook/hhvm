<?hh

class Props {
  public string $a;
  public static string $b;
}

function wrap($fun) {
  try {
    $fun();
  } catch (Exception $e) { echo "caught: ".$e->getMessage()."\n"; }
}

function foo(string $s) {
  var_dump($s);
  var_dump(is_string($s));
  var_dump(strlen($s));
}

function bar($f) {
  var_dump($f);
  var_dump(is_string($f));
  try { var_dump(strlen($f)); } catch (Exception $e) { var_dump($e->getMessage()); }
}

function baz(): string {
  return HH\fun('baz');
}

function buz() {
  return fun('buz');
}

function io(inout string $a, inout $b): string {
  var_dump($a, $b);
  list($a, $b) = varray[$b, $a];
  return $a;
}

function main() {
  wrap(() ==> foo("hello"));
  wrap(() ==> foo(HH\fun('foo')));

  wrap(() ==> bar("hello"));
  wrap(() ==> bar(HH\fun('bar')));

  wrap(() ==> var_dump(baz()));
  wrap(() ==> var_dump(is_string(baz())));
  wrap(() ==> var_dump(baz() is string));
  wrap(() ==> var_dump(baz() as string));
  wrap(() ==> var_dump(buz()));
  var_dump(is_string(buz()), buz() is string);
  wrap(() ==> var_dump(buz() as string));

  wrap(() ==> (new Props)->a = buz());
  wrap(() ==> Props::$b = buz());

  $x = HH\fun('foo');             wrap(()==>var_dump(io(inout $x, inout $x)));
  $y = 'foo';                     wrap(()==>var_dump(io(inout $y, inout $y)));
  $x = HH\fun('foo'); $y = 'foo'; wrap(()==>var_dump(io(inout $x, inout $y)));
  $x = HH\fun('foo'); $y = 'foo'; wrap(()==>var_dump(io(inout $y, inout $x)));
  var_dump($x, $y);
}
<<__EntryPoint>>
function main_entry(): void {

  set_error_handler(($_n, $str) ==> { throw new Exception($str); });

  for ($i = 0; $i < 10; $i++) main();
}
