<?hh

trait T {
  public function foo(): void {}
  public function bar(): void {}
}

trait T1 { use T; }
trait T2 { use T; }

trait TP {
  use T1, T2;

  public function bar(): void {}
} 
