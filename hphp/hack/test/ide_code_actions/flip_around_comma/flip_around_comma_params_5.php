<?hh

class Klass {
  public function __construct(
    // Don't suggest a "flip around comma" refactor for params where it's hard to calculate positions
    // Here it's hard to determine a position for "public".
      public int $a,
      int $b = 3,

      /*range-start*//*range-end*/?int $c = null
    ): void {}
}
