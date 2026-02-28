<?hh
async function foo() :AsyncGenerator<mixed,mixed,void>{
  yield 123;
}

<<__EntryPoint>>
function main_yield_inside_async_func() :mixed{
foo();
echo "Done\n";
}
