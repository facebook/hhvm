<?hh
class A {
    public static $b = 0;
    public static $c = vec[0, 1];
    public static $A_str = 'A';
}

<<__EntryPoint>>
function main_static_member() :mixed{
$A_str = 'A';
$A_obj = new A;
$b_str = 'b';
$c_str = 'c';
var_dump(A::$b);
var_dump($A_str::$b);
var_dump($A_obj::$b);
}
