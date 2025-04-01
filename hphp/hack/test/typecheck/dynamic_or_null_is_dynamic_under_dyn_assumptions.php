<?hh

function foo(MyObj $obj): void {
  \HH\Lib\Vec\map(
    $obj->getVecMixed(),
    ($x) ==> {
      $x = $x ?? get_nothing();
      if ($x is Traversable<_>) {
        $obj->takesTraversable($x);
      }
    },
  );
}

final class MyObj {
  public function takesTraversable(Traversable<mixed> $x): void {}
  public function getVecMixed(): vec<mixed> {
    return vec[];
  }
}

function get_nothing(): nothing {
  throw new Exception();
}
