<?hh

class C<reify Ta, reify Tb> {
  public static function f() {
    g<Ta>();
  }
}

<<__EntryPoint>> function main(): void {}
