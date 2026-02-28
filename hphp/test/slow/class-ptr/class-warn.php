<?hh

class Props {
  public string $a;
  public static string $b;
}

function wrap($fun) :mixed{
  try {
    $fun();
  } catch (Exception $e) { echo "caught: ".$e->getMessage()."\n"; }
}

<<__DynamicallyReferenced>> class foo {}
<<__DynamicallyReferenced>> class bar {}
<<__DynamicallyReferenced>> class baz {}
<<__DynamicallyReferenced>> class buz {}

function foo(string $s) :mixed{
  var_dump($s);
  var_dump(is_string($s));
  var_dump(strlen($s));
}

function bar($f) :mixed{
  var_dump($f);
  var_dump((string)$f);
  var_dump(is_string($f));
  try {
    var_dump(strlen($f));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

function baz(): string {
  return HH\classname_to_class('baz');
}

function buz() :mixed{
  return HH\classname_to_class('buz');
}

function io(inout string $a, inout $b): string {
  var_dump($a, $b);
  list($a, $b) = vec[$b, $a];
  return $a;
}

function main() :mixed{
  foo("hello");
  foo(HH\classname_to_class('foo'));

  bar("hello");
  bar(HH\classname_to_class('bar'));

  var_dump(baz(), is_string(baz()), baz() is string, baz() as string);
  var_dump(buz(), is_string(buz()), buz() is string, buz() as string);

  wrap(() ==> (new Props)->a = buz());
  wrap(() ==> Props::$b = buz());

  $x = HH\classname_to_class('foo');             var_dump(io(inout $x, inout $x));
  $y = 'foo';                     var_dump(io(inout $y, inout $y));
  $x = HH\classname_to_class('foo'); $y = 'foo'; var_dump(io(inout $x, inout $y));
  $x = HH\classname_to_class('foo'); $y = 'foo'; var_dump(io(inout $y, inout $x));
  var_dump($x, $y);
}
<<__EntryPoint>>
function main_entry(): void {

  set_error_handler(($n, $str) ==> {
    if ($n === E_RECOVERABLE_ERROR) throw new Exception($str);
    $fun = debug_backtrace()[1]['function'];
    echo "notice($fun): $str\n";
    return true;
  });

  for ($i = 0; $i < 10; $i++) main();
}
