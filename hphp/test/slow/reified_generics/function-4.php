<?hh

class C<reify T> {}
function c<reify T>(C<T> $x): void {
  echo "ok\n";
}

<<__EntryPoint>>
function f(): void {
  // TODO(T46022709): Handle variadic args
  c<(function(int):void)>(new C<(function(int, mixed...):void)>);
}
