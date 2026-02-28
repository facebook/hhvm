<?hh

function foo(?(int, int) $x) :mixed{}
<<__EntryPoint>> function main(): void {
foo(null);
foo(vec[1,2]);
foo(vec[1,2,3]); // ok: typechecker validates it
foo(new stdClass);
}
