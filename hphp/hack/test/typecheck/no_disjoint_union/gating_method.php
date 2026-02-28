<?hh

class C {
  public function no_disjoint_union_identity<<<__NoDisjointUnion>> T>(T $t): T {
    return $t;
  }
}
