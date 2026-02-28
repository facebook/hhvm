<?hh
class A {
   const FOO = vec[self::BAR];
   const BAR = vec[self::FOO];
}
<<__EntryPoint>> function main(): void {
var_dump(A::FOO);
}
