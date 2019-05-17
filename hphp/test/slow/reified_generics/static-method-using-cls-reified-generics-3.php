<?hh

class A{}

class C<reify Ta, reify Tb> {
  public static function f() {
    new A<Ta>();
  }
}

<<__EntryPoint>> function main(): void {}
