<?hh

class C<reify T> {}
function f(<<__Soft>> C<T> $x) { echo "done\n"; }

<<__EntryPoint>>
function main() {
  f(new C<int>());
}

