<?hh


namespace NSTest;

function f<T>(T $x, bool $b): int {
  echo "namespaces are fun\n";
  return 1;
}


<<__EntryPoint>>
function main_generic_1() :mixed{
f(1, true);
}
