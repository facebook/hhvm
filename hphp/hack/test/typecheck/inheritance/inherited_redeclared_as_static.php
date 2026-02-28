<?hh

class ABC {
  protected string $non_static_property = "";
  protected static string $static_property = "";

  protected function nonStaticMethod(): void {}
  protected static function staticMethod(): void {}
}

class DEF extends ABC {
  public static string $non_static_property = "";
  public string $static_property = "";

  protected static function nonStaticMethod(): void {}
  protected function staticMethod(): void {}
}
