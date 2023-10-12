<?hh // strict

class Basic {
  <<__Policied("I")>>
  public int $i = 0;
  <<__Policied("V")>>
  public vec<int> $v = vec[];

  <<__InferFlows>>
  public function set(): void {
    $this->v[] = $this->i;
  }

  <<__InferFlows>>
  public function mutation(): void {
    $this->v[0] += $this->i;
  }

  <<__InferFlows>>
  public function mutationKeyLeak(): void {
    $this->v[$this->i] = 42; // I leaks to V through the key
  }

  <<__InferFlows>>
  public function nested(vec<vec<int>> $vv): void {
    $vv[42][] = $this->v[0];

    $this->i = $vv[0][0];
  }
}

class COW {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("X")>>
    public int $x,
    <<__Policied("Y")>>
    public int $y,
    <<__Policied("VX")>>
    public vec<int> $vx,
  ) {}

  // copy-on-write semantics means $this->vx is vec[$x] and thus
  // does not depend on $this->y
  <<__InferFlows>>
  public function copyOnWrite(vec<int> $v): void {
    $v[] = $this->x;
    $this->vx = $v;
    $v[] = $this->y;
  }
}
