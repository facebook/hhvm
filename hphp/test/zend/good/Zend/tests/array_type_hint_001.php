<?hh
function foo(varray $a) :mixed{
    echo count($a)."\n";
}
<<__EntryPoint>> function main(): void {
foo(vec[1,2,3]);
foo(123);
}
