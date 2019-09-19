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
  return hh\fun('baz');
}

function buz() {
  return hh\fun('buz');
}

function io(inout string $a, inout $b): string {
  var_dump($a, $b);
  list($a, $b) = [$b, $a];
  return $a;
}

function main() {
  foo("hello");
  foo(hh\fun('foo'));

  bar("hello");
  bar(hh\fun('bar'));

  var_dump(baz(), is_string(baz()), baz() is string, baz() as string);
  var_dump(buz(), is_string(buz()), buz() is string, buz() as string);

  wrap(() ==> (new Props)->a = buz());
  wrap(() ==> Props::$b = buz());

  $x = hh\fun('foo');             var_dump(io(inout $x, inout $x));
  $y = 'foo';                     var_dump(io(inout $y, inout $y));
  $x = hh\fun('foo'); $y = 'foo'; var_dump(io(inout $x, inout $y));
  $x = hh\fun('foo'); $y = 'foo'; var_dump(io(inout $y, inout $x));
  var_dump($x, $y);
}
<<__EntryPoint>>
function main_entry(): void {

  set_error_handler(($_n, $str) ==> { throw new Exception($str); });

  for ($i = 0; $i < 10; $i++) main();
}
