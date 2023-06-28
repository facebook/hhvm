<?hh

trait T {
  static function f() :mixed{
    dict[ static::class => static::class ];
    echo "Ok\n";
  }
}
<<__EntryPoint>> function main(): void {
T::f();
}
