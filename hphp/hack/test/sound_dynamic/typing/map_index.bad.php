<?hh


function test_dyn_idx(
  dynamic $d,
  Map<int, int> $m1,
  Map<arraykey, int> $m2,
) : void {
  $m1[$d] = 1;

  $m1[$d] = "s";
  $m2[$d] = "s";
  $m1[$d] = $d;
  $m2[$d] = $d;
}

class C<Tk as arraykey> {
  public function __construct(
    private Map<Tk, int> $m1,
    private Map<arraykey, int> $m2,
    private Map<int, int> $m3,
  ) {}

  public function f(dynamic $d): void {
    $this->m1[$d] = 1;
    $this->m3[$d] = 1;

    $this->m1[$d] = "s";
    $this->m2[$d] = "s";
    $this->m3[$d] = "s";
    $this->m1[$d] = $d;
    $this->m2[$d] = $d;
    $this->m3[$d] = $d;
  }
}
