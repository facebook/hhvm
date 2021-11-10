<?hh

function test_dyn_index(
  dynamic $d,
  keyset<int> $ks1,
  keyset<arraykey> $ks2,
) : void {
  $a = $ks1[$d];
  hh_expect_equivalent<int>($a);
  $a = $ks2[$d];
  hh_expect_equivalent<arraykey>($a);
}

class C<Tk as arraykey> {
  public function __construct(
    private keyset<Tk> $ks1,
    private keyset<arraykey> $ks2,
    private keyset<int> $ks3,
  ) {}

  public function f(dynamic $d): void {
    $a = $this->ks1[$d];
    hh_expect_equivalent<Tk>($a);
    $a = $this->ks2[$d];
    hh_expect_equivalent<arraykey>($a);
    $a = $this->ks3[$d];
    hh_expect_equivalent<int>($a);
  }
}
