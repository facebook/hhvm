<?hh

    class C {
      private static $v;
      public static function f() {
        return self::$v;
      }
    }

$x = 1;
