<?hh

trait T {
  public function read(shape(nameof T => int) $s): int {
    return $s[nameof T];
  }

  public function idx(shape(nameof T => int) $s): ?int {
    return Shapes::idx($s, nameof T);
  }

  public function dict_read(dict<string, int> $d): int {
    return $d[nameof self];
  }
}
