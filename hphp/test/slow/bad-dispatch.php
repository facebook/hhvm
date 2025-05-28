<?hh

class Foo {
  <<__DynamicallyCallable>> function a() :mixed{}
  <<__DynamicallyCallable>> static function b() :mixed{}
}

<<__EntryPoint>>
function main() :mixed{
  $foo = 'Foo';
  $a = 'a';
  $b = 'b';

  (new Foo)->a();
  (new $foo)->$a();
  Foo::b();
  HH\dynamic_class_meth($foo, $b)();

  try {
    Foo::a();
  } catch (BadMethodCallException $e) {
    var_dump($e->getMessage());
  }

  try {
    HH\dynamic_class_meth($foo, $a)();
  } catch (InvalidArgumentException $e) {
    var_dump($e->getMessage());
  }

  try {
    (new Foo)->b();
  } catch (BadMethodCallException $e) {
    var_dump($e->getMessage());
  }

  try {
    (new $foo)->$b();
  } catch (BadMethodCallException $e) {
    var_dump($e->getMessage());
  }
}
