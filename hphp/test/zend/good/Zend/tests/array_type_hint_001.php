<?hh
function foo(array $a) {
    echo count($a)."\n";
}
<<__EntryPoint>> function main(): void {
foo(varray[1,2,3]);
foo(123);
}
