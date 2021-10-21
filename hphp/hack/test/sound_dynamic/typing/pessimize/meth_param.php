<?hh

class C {
  <<__SupportDynamicType>>
  public function f(shape() $s): void {
    $this->g($s);
  }

  <<__SupportDynamicType>>
  public static function ff(shape() $s): void {
    self::gg($s);
  }

  public function g(shape() $s): void {}

  public static function gg(shape() $s): void {}
}
