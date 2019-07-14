<?hh

class X {
  function y() {
    return static function() { return $this; };
  }
}

<<__EntryPoint>> function main(): void {}
