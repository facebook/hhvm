<?hh

class Bar {
  public function baz(): void {}

    public static function baz2(): void {}
}

function test(): void {
  Bar::baz<>;
}
