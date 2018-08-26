<?hh

trait T {
  static function f() {
    dict[ static::class => static::class ];
    echo "Ok\n";
  }
};

T::f();
