<?hh

class TheParent {
    const dict<string, string> DICT = dict["a" => "2"]; // expect ?'b' key
}

class TheChild extends TheParent {
    public static function foo(): void {
      parent::DICT['b'];  // out of bounds T136763758
    }
}
