<?hh

<<__EntryPoint>>
function main_second_argument() {
assert_options(ASSERT_ACTIVE,1);
assert_options(ASSERT_WARNING,1);

assert(false, "asserting that false is true");
assert(true, "foo");
assert(1===varray[], "This doesn't make any sense.\n\"");
}
