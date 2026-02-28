<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type MyInt as int = int;

enum E1 : int as MyInt {
  A = 1;
  B = 2;
}

enum E2 : MyInt {
  A = 1;
  B = 2;
}

enum class E3 : MyInt {
  MyInt A = 1;
  MyInt B = 2;
}
