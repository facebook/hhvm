<?hh // partial

final class C {
  public function __construct(private int $x, string &$y) {
    $y = 'bar';
  }
}
