<?hh

class Foo {
  public function __construct(private $options) {
    var_dump(debug_backtrace($this->options));
  }

  public function __destruct() {
    var_dump(debug_backtrace($this->options));
  }
}

function bar($options) {
  var_dump(debug_backtrace($options));
}

function foo($options) {
  HH\set_frame_metadata(new Foo($options));
  bar($options);
}

foo(0);
foo(DEBUG_BACKTRACE_PROVIDE_METADATA);
