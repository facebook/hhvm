<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type MyInt = int;

case type MyBool = bool;

function f(MyInt $x, MyBool $y): bool {
    return $x === $y;
}
