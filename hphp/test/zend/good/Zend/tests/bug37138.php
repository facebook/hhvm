<?hh
class st {
  <<__DynamicallyCallable>> public static function e() { echo ("EHLO\n"); }
  public static function e2() { call_user_func(varray[self::class, 'e']); }
}
class stch extends st {
  public static function g() { call_user_func(varray[parent::class, 'e']); }
}
<<__EntryPoint>> function main(): void {
  st::e();
  st::e2();
  stch::g();
}
