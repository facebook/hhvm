<?hh

namespace MyNS\Herp {
  function do_stuff() :mixed{
    \var_dump(__FUNCTION__);
  }
}

namespace MyNS\Derp {
  function do_stuff() :mixed{
    \var_dump(__FUNCTION__);
  }
}

namespace {
  use namespace MyNS\{Herp, Derp};
  <<__EntryPoint>> function main(): void {
    Herp\do_stuff();
    Derp\do_stuff();
  }
}
