<?hh

class A<+T1, -T2 as T1> {
  public function __construct() {}
  public function put(T2 $x): void {}
  public function get(): ?T1 {
    return null;
  }
}

function test(): void {
  $x = new A(); // x : A<v1, v2>, -v2 <: +v1. Either solve v1: v2 or v2: v1
  take_mixed($x->get()); // Either v2 <: mixed or v1 <: mixed
  take_arraykey_opt($x->get()); // Either v2 <: string or v1 <: string
  $x->put(0); // Either int <: v2 or int <: v1
  $x->put(""); // Either string <: v2 or string <: v1
  // All this should not produce any error, as there are at least
  //  a few solutions for v1 = v2:
  // (int | string), arraykey, ?(int|string), ?arraykey
}

function take_mixed(mixed $x): void {}
function take_arraykey_opt(?arraykey $x): void {}
