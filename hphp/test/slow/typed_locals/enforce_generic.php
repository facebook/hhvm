<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

class C<T1 as arraykey> {
  public function f<T2>(): void {
    let $x: dict<T1, T2>;
    $x = dict[1 => 2];
  }
}

class D<reify T1 as arraykey> {
  public function f<reify T2>(): void {
    let $x: dict<T1, T2>;
    $x = dict[1 => 2];
  }
}

class E1<reify T1> {}

function f<T>() {
  let $x: E1<T>;
  $x = new E1<int>();
}

<<__EntryPoint>>
function main(): void {
  $c = new C<string>();
  $c->f<string>();
  f<string>();
}
