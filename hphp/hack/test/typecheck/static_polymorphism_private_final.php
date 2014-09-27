<?hh // strict

final class A {
  private static int $a = 1;

  public function f(): int {
    return $this::$a;
  }
}
