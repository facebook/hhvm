<?hh

<<file:__EnableUnstableFeatures('case_types')>>

class A {}
class B {}

case type C1 = int | string | float | null;
case type C2 = bool | A;
case type C3 = B | C2;


<<__EntryPoint>>
function main() :mixed{
  $values = vec[1, "asd", 1.2, null, true, new A, new B];

  echo "C1\n";
  foreach ($values as $v) var_dump($v is C1);
  echo "\nC2\n";
  foreach ($values as $v) var_dump($v is C2);
  echo "\nC3\n";
  foreach ($values as $v) var_dump($v is C3);
}
