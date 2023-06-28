<?hh
class C {
  public static async function foo() :AsyncGenerator<mixed,mixed,void>{
    yield 123;
  }
}

<<__EntryPoint>>
function main_yield_inside_async_meth() :mixed{
C::foo();
echo "Done\n";
}
