<?hh

class C<reify T>  {}
class D<reify T1, reify T2, reify T3> extends C <(function(T1, T2):T3)> {}

function g<reify T>(C<T> $x): void {
  echo "ok\n";
}

<<__EntryPoint>>
function f(): void {
  g<(function(int, string):bool)>(new D<int, string, bool>());
  g<(function(int, float):bool)>(new D<int, string, bool>());
}
