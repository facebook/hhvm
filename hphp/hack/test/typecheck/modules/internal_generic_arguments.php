//// module_A.php
<?hh
new module A {}
//
//// module_B.php
<?hh
new module B {}
//
//// A.php
<?hh
module A;

internal class T {
  public static function g(): F<int> {
    return new F();
  }
  public static function h(F<string> $_): void {
  }
}

internal class F<T> {
}

function test(): void {
  T::h(T::g()); // This will error
}

final class C extends T {
}

//// B.php
<?hh
module B;
async function main(): Awaitable<void> {
  C::h(C::g()); // This should also error, even though F is treated opaquely here
}
