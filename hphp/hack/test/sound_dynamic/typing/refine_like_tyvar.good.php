<?hh

<<__SupportDynamicType>>
interface I1 {}

<<__SupportDynamicType>>
interface I2 extends I1 {
  public function m() : void;
}

<<__SupportDynamicType>>
interface I3 extends I1 {
  public function m() : void;
}

function getI(): vec<I1> {
  return vec[];
}

function map_and_filter_nulls<Tv1 , Tv2 >(
  Traversable<Tv1> $traversable,
  (function(Tv1): ~?Tv2) $value_func,
): vec<Tv2> {
  return vec[];
}

function f(): void {
  $x = getI()
    |> map_and_filter_nulls(
      $$,
      $y ==> {
      if ($y is I2 || $y is I3) {
        $z = $y;
      }
      else {
        $z = null;
      }

      return $z;
    }
    );
  $x[0]->m();
}
