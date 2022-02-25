<?hh

function test_dyn_index(
  dynamic $d,
  dict<int, int> $dt1,
  dict<arraykey, int> $dt2,
) : void {
  $a = $dt1[$d];
  hh_expect_equivalent<int>($a);
  $a = $dt2[$d];
  hh_expect_equivalent<int>($a);

  $dt1[$d] = 1;
  $dt1[$d] = "s";
  hh_expect_equivalent<dict<((arraykey&dynamic)|int), (int|string)>>($dt1);
  $dt2[$d] = $d;
  $dt2[$d] = 1;
  $dt2[$d] = "s";
  $dt2[$d] = $d;
}

class C<Tk as arraykey> {
  public function __construct(
    private dict<Tk, int> $dt1,
    private dict<arraykey, int> $dt2,
    private dict<int, int> $dt3,
  ) {}

  public function f(dynamic $d): void {
    $a = $this->dt1[$d];
    hh_expect_equivalent<int>($a);
    $a = $this->dt2[$d];
    hh_expect_equivalent<int>($a);
    $a = $this->dt3[$d];
    hh_expect_equivalent<int>($a);

    $this->dt2[$d] = 1;
  }
}
