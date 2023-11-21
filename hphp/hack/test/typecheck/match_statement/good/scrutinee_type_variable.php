<?hh

<<file:__EnableUnstableFeatures('match_statements')>>
<<file:__EnableUnstableFeatures('case_types')>>

class A {}
class B {}
class C {}
class D {}
case type AB = A | B;
case type CD = C | D;

function f (AB $ab, CD $cd): void {
  $vec = vec[$ab, $cd];
  foreach($vec as $x) {
    match ($x) {
      _: A => print("A");
      _: B => print("B");
      _: C => print("C");
      _: D => print("D");
    }
  }
}
