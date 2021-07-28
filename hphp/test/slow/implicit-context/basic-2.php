<?hh

function printImplicit() {
  var_dump(IntContext::getContext());
}

function addFive() {
  return HH\Lib\Legacy_FIXME\cast_for_arithmetic(IntContext::getContext())
    + HH\Lib\Legacy_FIXME\cast_for_arithmetic(IntContext::start(4, printImplicit<>));
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';

  var_dump(IntContext::start(5, addFive<>));
}
