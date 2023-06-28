<?hh

<<file:__EnableUnstableFeatures('case_types')>>

class A<reify T> {}

case type C1 = int | string;
case type C2 = int | string;

<<__EntryPoint>>
function main() :mixed{
  var_dump((new A<C1>) is A<C1>); // true
  // false because it should use the type as witness
  var_dump((new A<C1>) is A<C2>);
}
