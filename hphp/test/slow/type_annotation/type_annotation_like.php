<?hh

class C {
  const type A = ~int;
  const type B = vec<~int>;
  const type C = ~dict<string, int>;
  const type D = (int, ~float, bool);
  const type E = shape('x' => string, 'y' => vec<~int>);
  const type F = (function (): ~int);
  const type G = ~this::F;
  const type H = ?~int;
  const type I = ~C;
  const type J = ~~int;
}

function f(string $const_name): void {
  echo "=== ".$const_name." ===\n";

  $x = new ReflectionTypeConstant(C::class, $const_name);
  var_dump($x->getAssignedTypeText());
  var_dump($x->getTypeStructure());
}

<<__EntryPoint>>
function main() :mixed{
  f('A');
  f('B');
  f('C');
  f('D');
  f('E');
  f('F');
  f('G');
  f('H');
  f('I');
  f('J');
}
