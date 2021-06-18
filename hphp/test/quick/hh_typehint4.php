<?hh

function foo(?(int, int) $x) {}
<<__EntryPoint>> function main(): void {
foo(null);
foo(varray[1,2]);
foo(varray[1,2,3]); // ok: typechecker validates it
foo(new stdClass);
}
