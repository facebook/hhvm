<?hh

trait T1 {
  public final function f(): void {}
}
trait T2 {
  use T1;
}
trait T3 {
  use T1;
  use T2;
}
