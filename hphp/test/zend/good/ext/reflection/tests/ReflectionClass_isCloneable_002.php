<?hh

trait foo {
}

abstract class bar {
}

interface baz {
}

<<__EntryPoint>>
function main_entry(): void {
  $obj = new ReflectionClass('foo');
  var_dump($obj->isCloneable());
  $obj = new ReflectionClass('bar');
  var_dump($obj->isCloneable());
  $obj = new ReflectionClass('baz');
  var_dump($obj->isCloneable());
}
