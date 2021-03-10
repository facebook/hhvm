<?hh

function myErrorHandler($errno, $errstr, $errfile, $errline) {
  echo "Error: $errstr at $errfile($errline)\n";
  throw new Exception();
}

class A {}
class B {}

function foo1(classname $a) : classname {
  var_dump($a);
  return $a;
}

class Bar1 {
  public classname $name = A::class;
}

type MyType = HH\classname;

function foo2(MyType $a) : MyType {
  var_dump($a);
  return $a;
}

class Bar2 {
  public MyType $name = A::class;
}

function mymain() {
  set_error_handler(myErrorHandler<>, E_RECOVERABLE_ERROR);

  var_dump(foo1(A::class));
  $o1 = new Bar1();
  $o1->name = B::class;
  var_dump($o1->name);

  var_dump(foo2(A::class));
  $o2 = new Bar2();
  $o2->name = B::class;
  var_dump($o2->name);

  var_dump(foo1('A'));
  $o1 = new Bar1();
  $o1->name = 'B';
  var_dump($o1->name);

  var_dump(foo2('A'));
  $o2 = new Bar2();
  $o2->name = 'B';
  var_dump($o2->name);

  try { foo1(1); } catch (Exception $e) {}
  try { foo2(1); } catch (Exception $e) {}
  try { $o1->name = 1; } catch (Exception $e) {}
  try { $o2->name = 1; } catch (Exception $e) {}
}
