<?hh

class Baz {
  <<__NEVER_INLINE>>
  private function bar($options) :mixed{
    var_dump(debug_backtrace($options));
  }

  <<__NEVER_INLINE>>
  public function foo($options) :mixed{
    HH\set_frame_metadata('I am foo');
    $this->bar($options);
  }
}

<<__EntryPoint>>
function main(): void {
  $baz = new Baz();
  $baz->foo(DEBUG_BACKTRACE_PROVIDE_OBJECT);
  $baz->foo(DEBUG_BACKTRACE_PROVIDE_OBJECT|DEBUG_BACKTRACE_PROVIDE_METADATA);
}
