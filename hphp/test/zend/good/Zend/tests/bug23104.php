<?hh
function foo($bar = array("a", "b", "c"))
{
    var_dump(current($bar));
}
<<__EntryPoint>> function main(): void {
foo();
}
