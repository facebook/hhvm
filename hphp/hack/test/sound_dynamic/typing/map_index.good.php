<?hh

function test_dyn_index(
  dynamic $d,
  Map<int, int> $m1,
  Map<arraykey, int> $m2,
) : void {
  $a = $m1[$d];
  hh_expect_equivalent<~int>($a);
  $a = $m2[$d];
  hh_expect_equivalent<~int>($a);

  $m2[$d] = 1;
}

class C<Tk as arraykey> {
  public function __construct(
    private Map<Tk, int> $m1,
    private Map<arraykey, int> $m2,
    private Map<int, int> $m3,
  ) {}

  public function f(dynamic $d): void {
    $a = $this->m1[$d];
    hh_expect_equivalent<~int>($a);
    $a = $this->m2[$d];
    hh_expect_equivalent<~int>($a);
    $a = $this->m3[$d];
    hh_expect_equivalent<~int>($a);

    $this->m2[$d] = 1;
  }
}
