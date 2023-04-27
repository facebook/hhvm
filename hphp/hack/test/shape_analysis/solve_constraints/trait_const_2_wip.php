<?hh

class TheParent {
    // T136213776: expect ?'b' key
    const dict<string, string> DICT = dict["a" => "2"];
}

trait Childish {
    require extends TheParent;
    public static function foo(): void {
      parent::DICT['b'];
    }
}

class Child extends TheParent {
  use Childish;
}
