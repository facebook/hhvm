<?hh

namespace Herp\Derp {
  function do_stuff(): void {
    var_dump(__FUNCTION__);
  }
}

namespace {
  // Config defines c as \foo\bar
  use namespace \Herp\Derp as c;
  c\do_stuff();
}
