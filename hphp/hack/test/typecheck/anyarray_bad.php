<?hh

function takes_array(AnyArray<int, int> $_): void {}
function gives_array(): AnyArray<int, int> { return vec[]; }
function handles_generic_array<Tk as arraykey, Tv>(
  AnyArray<Tk, Tv> $a,
): AnyArray<Tk, Tv> { return $a; }

function does_things(): void {
  takes_array(Map {}); // error
  takes_array(Set {}); // error
  takes_array(ImmVector {}); // error
  handles_generic_array<int, string>(gives_array()); // error

  $a = gives_array();
  $a[42];
  $a[42] = 43; // error
}

interface Parent_ {
  public function foo(AnyArray<arraykey, int> $f): void;
}

class Child implements Parent_ {
  public function foo(AnyArray<int, int> $f): void {} // error
}

function fromArrays(mixed ...$argv): Set<arraykey> {
  $ret = Set {};
  foreach ($argv as $arr) {
    if (!HH\is_any_array($arr)) {
      return Set {};
    }
    hh_show($arr);
    $ret->addAll($arr);
  }
  return $ret; // error
}
