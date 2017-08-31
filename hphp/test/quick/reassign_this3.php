<?hh
function x($this){} // allow
class Foo {
  static function x($this){} // allow
  public function y($this){} // error
}
