<?hh
async function foo() {
  yield 123;
}

<<__EntryPoint>>
function main_yield_inside_async_func() {
foo();
echo "Done\n";
}
