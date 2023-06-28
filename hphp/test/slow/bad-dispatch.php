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
  $foo::$b();

  try {
    Foo::a();
  } catch (BadMethodCallException $e) {
    var_dump($e->getMessage());
  }

  try {
    $foo::$a();
  } catch (BadMethodCallException $e) {
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
