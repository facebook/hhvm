<?hh

trait T {
  public function read(shape(nameof T => int) $s): int {
    return $s[nameof self];
  }

  public function write(shape(...) $s): void {
    $s[nameof self] = 42;
  }
}
