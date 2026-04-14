<?hh

class C {
  public function __construct(private int $x) {}

  public static function make(): void {
    $y = 42;
    new static($y);
//             ^ enforcement-at-caret
  }
}
