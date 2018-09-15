<?hh // strict

// OK
function nonReact0(): void {
  $lambda = (Vector<int> $x) ==> {
    $x[] = 5;
  };
}

// OK
function nonReact1(): void {
  $lambda = (Set<int> $x) ==> {
    $x[] = 5;
  };
}

// OK
function nonReact2(): void {
  $lambda = (Map<int, int> $x) ==> {
    $x[] = Pair { 1, 2 };
  };
}

class A {
  public function __construct(public int $v) {}
}

// OK
function nonReact3(): void {
  $lambda = (A $x) ==> {
    $x->v = 5;
  };
}

// OK
function nonReact4(): void {
  $lambda = () ==> {
    $z = new A(5);
    $z1 = $z;
  };
}
