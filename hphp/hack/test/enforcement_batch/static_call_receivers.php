<?hh

class Base {
  public static function takes_int(int $_x): void {}
}

class Child extends Base {
  public static function call_via_self(): void {
    self::takes_int(42);
//                  ^ enforcement-at-caret
  }

  public static function call_via_static(): void {
    static::takes_int(42);
//                    ^ enforcement-at-caret
  }

  public static function call_via_parent(): void {
    parent::takes_int(42);
//                    ^ enforcement-at-caret
  }

  public static function call_via_classname(): void {
    Base::takes_int(42);
//                  ^ enforcement-at-caret
  }
}
