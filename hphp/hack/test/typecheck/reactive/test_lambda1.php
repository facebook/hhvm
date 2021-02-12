<?hh //strict
function normal(string $s): int {
  return 1;
}


function rx1(): void {
  // OK - if lambda is not called
  $l = <<__NonRx>>() ==> {
    normal("text");
  };
}

class C {
  public function __construct(public int $v) {}
}


function rx2(): void {
  // OK - if lambda is not called
  $l = <<__NonRx>>(bool $a, C $c) ==> {
    if ($a) {
      normal("text1");
    } else {
      $c->v = 1;
    }
  };
}
