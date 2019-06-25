<?hh
class A {
   const FOO = [self::BAR];
   const BAR = [self::FOO];
}
<<__EntryPoint>> function main(): void {
var_dump(A::FOO);
}
