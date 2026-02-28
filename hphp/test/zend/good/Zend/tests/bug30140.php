<?hh
class A {
    public static $test1 = true;
    public static $test2 = vec[];
    public static $test3 = "str";
}

class B extends A {
}
<<__EntryPoint>> function main(): void {
A::$test1 = "x";
A::$test2 = "y";
A::$test3 = "z";
var_dump(A::$test1);
var_dump(A::$test2);
var_dump(A::$test3);
var_dump(B::$test1);
var_dump(B::$test2);
var_dump(B::$test3);
}
