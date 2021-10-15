<?hh

class C<Tk as arraykey> {
  public function __construct(
    private dict<Tk, int> $dt1,
    private dict<arraykey, int> $dt2,
    private dict<int, int> $dt3,
  ) {}

  public function f(dynamic $d): void {
    $this->dt1[$d] = 1;
    $this->dt3[$d] = 1;

    $this->dt1[$d] = "s";
    $this->dt2[$d] = "s";
    $this->dt3[$d] = "s";
    $this->dt1[$d] = $d;
    $this->dt2[$d] = $d;
    $this->dt3[$d] = $d;
  }
}
