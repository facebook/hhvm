<?hh
class C {
  public static async function foo() {
    yield 123;
  }
}
C::foo();
echo "Done\n";
