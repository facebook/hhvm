<?hh

class Box<T> {
  public function put(T $x): void {}
  public function get(): T { throw new Exception(); }
}

function pull<Tk, Tv1, Tv2>(
  Tv1 $input,
  (function(Tv1): Tv2) $value_func,
  (function(Tv1): Tk) $key_func,
): void {
  throw new Exception();
}

interface A1 {}
interface A2 {}
interface B1 {}
interface B2 {}

function test((A1, A2) $t1, (B1, B2) $t2): void {
  $x = new Box(); // Box<#1>
  $x->put($t1);
  $x->put($t2);
  $input = $x->get(); // {(A1, A2), (B1, B2)} <: #1

  pull(
    $input,
    $in ==> {
      list(
        $v1, // A1 <: #3 /\ B1 <: #3
        $v2  // A2 <: #4 /\ B2 <: #4
      ) = $in; // (A1, A2) <: list(#3, #4) /\ (B1, B2) <: list(#3, #4);
    },
    $in ==> {
      list($v1, $v2) = $in; // same as above
      hh_force_solve();
      hh_show($v1);
      hh_show($v2);
    },
  );
}
