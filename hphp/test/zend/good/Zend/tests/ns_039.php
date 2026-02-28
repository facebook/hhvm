<?hh
function foo($a = A) :mixed{
    echo "$a\n";
}
function bar($a = dict[A => B]) :mixed{
    foreach ($a as $key => $val) {
        echo "$key\n";
        echo "$val\n";
    }
}
const A = "ok";
const B = A;
<<__EntryPoint>> function main(): void {
echo A . "\n";
echo B . "\n";
foo();
bar();
}
