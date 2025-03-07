<?hh

class A<T> {}

class C {
  public static function foo(): A<int> {
    return new A();
  }
}

function main(): void {
  $obj = C::foo();
  if ($obj) {
    echo 'hello';
  }
}
