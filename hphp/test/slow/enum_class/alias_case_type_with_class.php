<?hh

<<file: __EnableUnstableFeatures('case_types')>>

function takes(CT_0 $_): void {}

<<__EntryPoint>>
function main(): void {
  $c = new C_2();
  takes($c);
}

// Auxiliary definitions
case type CT_0 = CT_1;
case type CT_1 = CT_2;
case type CT_2 = A_4 | E_4;

class C_1 {}
class C_2 {}
type A_4 = C_1;
enum E_4: C_2 {}
