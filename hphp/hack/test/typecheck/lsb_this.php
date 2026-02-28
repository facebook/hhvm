<?hh

<<__ConsistentConstruct>>
class C {
  <<__LSB>> private static ?this $instance = null;

  public static function get(): this {
    if (static::$instance === null) {
      static::$instance = new static();
    }
    return static::$instance;
  }
}

class D extends C {
  public static function get2(): D {
    return self::get();
  }
}
