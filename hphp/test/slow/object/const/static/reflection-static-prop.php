<?hh

class A {
  static int $bar = 20;
  <<__Const>> static int $foo = 24;
}

<<__EntryPoint>> function main(): void {
  $rb = new ReflectionClass("A");

  // Non-const Static property
  $rb->setStaticPropertyValue('bar', 50);
  var_dump($rb->getStaticPropertyValue('bar'));

  // Const Static property
  try {
    var_dump($rb->setStaticPropertyValue('foo', 55));
  } catch ( Exception $e ) {
    echo $e->getMessage() . "\n";
  }
  var_dump($rb->getStaticPropertyValue('foo'));
}
