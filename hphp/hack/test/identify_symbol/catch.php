<?hh

function might_throw(): void {}

class Foo extends Exception {}

function test() {
  try {
    might_throw();
  } catch (Foo $foo) {

  }
}
