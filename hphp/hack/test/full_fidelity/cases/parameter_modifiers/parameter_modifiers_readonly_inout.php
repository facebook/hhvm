<?hh

interface I1 {
  public function ri1(inout readonly int $x): void;
  public function ri2(readonly inout int $x): void;
}
