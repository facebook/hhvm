<?hh

<<file:__EnableUnstableFeatures('case_types')>>

class A<reify T> {}

case type C1 = int | string;
case type C2 = int | string;

type T1 = C1;
newtype NT1 = C1;
case type C3 = C1;

<<__EntryPoint>>
function main() :mixed{
  $a = new A<C1>();
  // True: Identical
  var_dump($a is A<C1>);
  // False: It should use the type as witness
  var_dump($a is A<C2>);
  // True: Identical through an indirection
  var_dump($a is A<T1>);
  // True: Same as regular alias since HHVM does not see newtype
  var_dump($a is A<NT1>);
  // False: Same reason as why C2 is not same as C1
  var_dump($a is A<C3>);
}
