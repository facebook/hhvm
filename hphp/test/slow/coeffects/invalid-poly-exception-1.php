<?hh

class C { const ctx C = [leak_safe]; }

function f(mixed $f)[ctx $f] :mixed{ echo "in f\n"; }
function g<reify T>()[T::C] :mixed{ echo "in g\n"; }

<<__EntryPoint>>
function main() :mixed{
  try { f(g<int>); } catch (Exception $e) { var_dump($e->getMessage()); }
}
