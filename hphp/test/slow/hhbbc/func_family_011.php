<?hh

abstract class Unimpl {
  static abstract function aa($x):mixed;

  static function go() :mixed{
    return self::doweirdthings();
  }

  static function doweirdthings() :mixed{
    $k = vec[];
    return static::aa($k);
  }
}

abstract class B extends Unimpl {
  static function aa($x) :mixed{
    return "hi\n";
  }

  static abstract function bb($x):mixed;

  static function doit() :mixed{
    parent::go();
  }
}


<<__EntryPoint>>
function main_func_family_011() :mixed{
B::doit("asd");
}
