<?hh
class A {
   const FOO = varray[self::BAR];
   const BAR = varray[self::FOO];
}
<<__EntryPoint>> function main(): void {
var_dump(A::FOO);
}
