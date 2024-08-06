<?hh

<<file: __EnableUnstableFeatures('case_types')>>

function takes(CT_0 $_): void {}

<<__EntryPoint>>
function main(): void {
  takes(E_3::A);
}

// Auxiliary definitions
case type CT_0 = CT_1;
case type CT_1 = CT_2;
case type CT_2 = A_4 | ?E_3;
type A_4 = C_5;
class C_5 {}
enum E_3: int { A = 42; }
