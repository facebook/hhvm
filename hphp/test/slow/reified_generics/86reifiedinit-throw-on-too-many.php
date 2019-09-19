<?hh

class D {}
class C<reify T> extends D {}

<<__EntryPoint>>
function main() {
  new C<int>();
  echo "done\n";
}
