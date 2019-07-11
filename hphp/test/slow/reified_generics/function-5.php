<?hh

class C<reify T> {}
function c<reify T>(C<T> $x): void {
  echo "ok\n";
}

function f<reify T1, reify T2>() {
  c<(function(?string):int)>(new C<(function(T1):T2)>);
}

<<__EntryPoint>>
function g(): void {
  f<?string, int>();
  f<null, int>();
  f<int, string>();
}
