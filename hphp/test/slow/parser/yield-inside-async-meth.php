<?hh
class C {
  public static async function foo() {
    yield 123;
  }
}

<<__EntryPoint>>
function main_yield_inside_async_meth() {
C::foo();
echo "Done\n";
}
