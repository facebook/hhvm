<?hh
async function foo() {
  yield 123;
}
foo();
echo "Done\n";
