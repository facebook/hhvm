<?hh

trait T {
  require extends D;

  public function foo(): int {
    return parent::bar();
  }
}
