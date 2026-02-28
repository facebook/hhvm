<?hh

class C {}

<<__EntryPoint>>
function main() {
  $_ = new ReflectionClass(C::class);
  echo "No implicit conversion logs should appear in this output.\n";
}
