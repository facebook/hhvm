<?hh

function get_float(): float;

function get_void_no_attr(): void;

<<__NativeData>>
class Foobar {

  public function getNull(): null;

  abstract public function getNothing(): nothing;

}

<<__NativeData>>
abstract class Baz {

  public function getNull(): null;

}
