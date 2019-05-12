<?hh

function foo(@(int, int) $x) {}
<<__EntryPoint>> function main(): void {
foo(null);
foo(array(1,2));
foo(array(1,2,3)); // ok: typechecker validates it
foo(new stdclass);
}
