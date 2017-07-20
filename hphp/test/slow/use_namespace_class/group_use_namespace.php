<?hh

namespace MyNS\Herp {
  function do_stuff() {
    var_dump(__FUNCTION__);
  }
}

namespace MyNS\Derp {
  function do_stuff() {
    var_dump(__FUNCTION__);
  }
}

namespace {
  use namespace MyNS\{Herp, Derp};
  Herp\do_stuff();
  Derp\do_stuff();
}
