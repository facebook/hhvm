<?hh

final class Foo extends Exception {}

type Alias = Foo;

function test(): void {
  try {}
  catch (Foo $e) {
  }
}

function test_alias(): void {
  try {}
  catch (Alias $e) {
  }
}

function test_unbound(): void {
  try {}
  catch (NotFoundException $e) {
  }
}

function test_generic<T as Exception>(): void {
  try {}
  catch (T $e) {
  }
}
