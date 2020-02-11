<?hh
function foo($bar = varray["a", "b", "c"])
{
    var_dump(current($bar));
}
<<__EntryPoint>> function main(): void {
foo();
}
