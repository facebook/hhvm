<?hh // strict

class Basic {
  <<Policied("I")>>
  public int $i = 0;
  <<Policied("V")>>
  public vec<int> $v = vec[];

  public function set(): void {
    $this->v[] = $this->i;
  }

  public function mutation(): void {
    $this->v[0] += $this->i;
  }

  public function mutationKeyLeak(): void {
    $this->v[$this->i] = 42; // I leaks to V through the key
  }

  public function nested(vec<vec<int>> $vv): void {
    $vv[42][] = $this->v[0];

    $this->i = $vv[0][0];
  }
}

class COW {
  public function __construct(
    <<Policied("X")>>
    public int $x,
    <<Policied("Y")>>
    public int $y,
    <<Policied("VX")>>
    public vec<int> $vx,
  ) {}

  // copy-on-write semantics means $this->vx is vec[$x] and thus
  // does not depend on $this->y
  //
  // This test currently does not pass; the analysis detects the
  // spurious flow
  public function copyOnWrite(vec<int> $v): void {
    $v[] = $this->x;
    $this->vx = $v;
    $v[] = $this->y;
  }
}
