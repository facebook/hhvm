<?hh

class C<reify T> {}

function c<reify T>(C<T> $x): void {
  echo "ok\n";
}

<<__EntryPoint>>
function f(): void {
  c<(function():int)>(new C<(function():int)>);
  c<(int, (function():int))>(new C<(int, (function():int))>);
}
