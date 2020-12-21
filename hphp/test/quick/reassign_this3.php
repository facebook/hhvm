<?hh
function x($this){} // allow
class Foo {
  static function x($this){} // allow
  public function y($this){} // error
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
