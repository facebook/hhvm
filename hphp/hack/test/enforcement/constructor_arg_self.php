<?hh

class C {
  public function __construct(private int $x) {}

  public static function make(): C {
    $y = 42;
    return new self($y);
//                  ^ enforcement-at-caret
  }
}
