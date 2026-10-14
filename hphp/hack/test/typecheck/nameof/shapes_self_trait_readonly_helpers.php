<?hh

trait T {
  public function idx(readonly shape(...) $s): void {
    $_ = readonly HH\Readonly\Shapes::idx($s, nameof self);
  }

  public function at(readonly shape(...) $s): void {
    $_ = readonly HH\Readonly\Shapes::at($s, nameof self);
  }
}
