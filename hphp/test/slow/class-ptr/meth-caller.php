<?hh

class C {
  function f(): void {
    echo "hello\n";
  }
}

<<__EntryPoint>>
function main(): void {
  $f = meth_caller(C::class, 'f');
  $f(new C());
}
