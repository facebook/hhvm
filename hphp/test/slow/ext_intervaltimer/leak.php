<?hh

final class C {
  private IntervalTimer $t;

  public function __construct() {
    $this->t = new IntervalTimer(1000.0, 1000.0, () ==> { $this->t->stop(); });
    $this->t->start();
  }
}

<<__EntryPoint>>
function test(): void {
  new C();
  echo "Done\n";
}
