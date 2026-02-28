<?hh

class C<reify T> {}
function f(<<__Soft>> C<T> $x) :mixed{ echo "done\n"; }

<<__EntryPoint>>
function main() :mixed{
  f(new C<int>());
}

