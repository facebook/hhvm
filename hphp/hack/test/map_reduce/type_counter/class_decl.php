<?hh

class C {
  public vec<int> $v = vec[];
  public int $i = 42;
  public dict<string,int> $d = dict[];

  public function m1(): vec<int> { return vec[]; }
  public function m2(): int { return 42; }
  public function m3(): dict<string,int> { return dict[]; }
}
