<?hh

class Foo {
    const dict<string, string> DICT = dict["a" => "2"]; // expect ?'b' key
    public static function foo(): void {
      static::DICT['b'];
    }
}
