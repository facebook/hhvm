<?hh

abstract class Unimpl {
  static abstract function aa($x);

  static function go() {
    return self::doweirdthings();
  }

  static function doweirdthings() {
    $k = varray[];
    return static::aa($k);
  }
}

abstract class B extends Unimpl {
  static function aa($x) {
    return "hi\n";
  }

  static abstract function bb($x);

  static function doit() {
    parent::go();
  }
}


<<__EntryPoint>>
function main_func_family_011() {
B::doit("asd");
}
