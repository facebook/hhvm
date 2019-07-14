<?hh

class X {
  static function y() {
    return function() { return $this; };
  }
}

<<__EntryPoint>> function main(): void {}
