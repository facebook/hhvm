<?hh

class D {}
class C<reify T> extends D {}

<<__EntryPoint>>
function main() :mixed{
  new C<int>();
  echo "done\n";
}
