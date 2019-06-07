<?hh
class A {
   const FOO = [self::BAR];
   const BAR = [self::FOO];
}
<<__EntryPoint>> function main() {
var_dump(A::FOO);
}
