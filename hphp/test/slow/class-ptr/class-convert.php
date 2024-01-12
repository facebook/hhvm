<?hh

class Props {
  public string $a;
  public static string $b;
}

class foo {}
class bar {}
class baz {}
class buz {}

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
  return __hhvm_intrinsics\create_class_pointer('baz');
}

function buz() :mixed{
  return __hhvm_intrinsics\create_class_pointer('buz');
}

function io(inout string $a, inout $b): string {
  var_dump($a, $b);
  list($a, $b) = vec[$b, $a];
  return $a;
}

function main() :mixed{
  foo("hello");
  foo(__hhvm_intrinsics\create_class_pointer('foo'));

  bar("hello");
  bar(__hhvm_intrinsics\create_class_pointer('bar'));

  var_dump(baz(), is_string(baz()), baz() is string, baz() as string);
  var_dump(buz(), is_string(buz()), buz() is string, buz() as string);

  $f1 = () ==> (new Props)->a = buz();
  $f1();
  $f2 = () ==> Props::$b = buz();
  $f2();

  $x = __hhvm_intrinsics\create_class_pointer('foo');             var_dump(io(inout $x, inout $x));
  $y = 'foo';                     var_dump(io(inout $y, inout $y));
  $x = __hhvm_intrinsics\create_class_pointer('foo'); $y = 'foo'; var_dump(io(inout $x, inout $y));
  $x = __hhvm_intrinsics\create_class_pointer('foo'); $y = 'foo'; var_dump(io(inout $y, inout $x));
  var_dump($x, $y);
}
<<__EntryPoint>>
function main_entry(): void {

  for ($i = 0; $i < 10; $i++) main();
}
