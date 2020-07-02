<?hh // strict

function empty(): vec<int> {
  $vec = vec[];
  return $vec;
}

function testCollection(): vec<string> {
  $there = "there";
  $vec = vec["hi", $there];
  return $vec;
}

function testAdd1(vec<int> $vec): void {
  $vec[] = 2;
}

function testAdd2(): void {
  $vec = vec[0,1];
  $vec[] = 2;
}

function retElem(): int {
  $vec = vec[1,2,3];
  return $vec[1];
}

class C {
  <<Policied("INT")>>
  public int $i = 0;
  <<Policied("VEC")>>
  public vec<int> $v = vec[];

  public function set(): void {
    $this->v[] = $this->i;
  }

  public function nested(vec<vec<int>> $vv): void {
    $vv[42][] = $this->v[0];

    $this->i = $vv[0][0];
  }

  public function mutation(): void {
    $this->v[0] += $this->i;
  }
}

function copyOnWrite(vec<int> $v, int $x, int $y): void {
  $v[] = $x;
  $vx = $v;
  $v[] = $y;
  // copy-on-write semantics means $vx is vec[$x]
  // and thus does not depend on $y
}
