<?hh
namespace X;
use X as Y;
const A = "ok\n";
const B = A;
function f1($x=A) :mixed{
    echo $x;
}
function f2($x=\X\A) :mixed{
    echo $x;
}
function f3($x=Y\A) :mixed{
    echo $x;
}
function f4($x=\X\A) :mixed{
    echo $x;
}
function f5($x=B) :mixed{
    echo $x;
}
function f6($x=vec[A]) :mixed{
    echo $x[0];
}
function f7($x=dict["aaa"=>A]) :mixed{
    echo $x["aaa"];
}
function f8($x=dict[A=>"aaa\n"]) :mixed{
    echo $x["ok\n"];
}
<<__EntryPoint>> function main(): void {
echo A;
echo \X\A;
echo Y\A;
echo \X\A;
f1();
f2();
f3();
f4();
echo B;
f5();
f6();
f7();
f8();
}
