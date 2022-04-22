<?hh

function get_float(): float;

function get_void_no_attr(): void;

<<__NativeData('Foobar')>>
class Foobar {

  public function getNull(): null;

  abstract public function getNothing(): nothing;

}

<<__NativeData('Baz')>>
abstract class Baz {

  public function getNull(): null;

}
