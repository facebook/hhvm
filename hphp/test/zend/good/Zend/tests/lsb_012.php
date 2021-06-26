<?hh

class Test1 {
  <<__DynamicallyCallable>> static function ok() {
    echo "bug";
  }
  static function test() {
    call_user_func(static::class."::ok");
  }
}

class Test2 extends Test1 {
  <<__DynamicallyCallable>> static function ok() {
    echo "ok";
  }
}
<<__EntryPoint>> function main(): void {
Test2::test();
}
