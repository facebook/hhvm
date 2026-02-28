<?hh
class st {
  <<__DynamicallyCallable>> public static function e() :mixed{ echo ("EHLO\n"); }
  public static function e2() :mixed{ call_user_func(vec[self::class, 'e']); }
}
class stch extends st {
  public static function g() :mixed{ call_user_func(vec[parent::class, 'e']); }
}
<<__EntryPoint>> function main(): void {
  st::e();
  st::e2();
  stch::g();
}
