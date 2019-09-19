<?hh

class C<reify T> {}
function c<reify T>(C<T> $x): void {
  echo "ok\n";
}

<<__EntryPoint>>
function f(): void {
  c<(function(int):void)>(new C<(function(int, mixed...):void)>);
}
