<?hh

include 'implicit.inc';

function f() {
  try {
    ClassContext::getContext();
  } catch (TypeAssertionException $_) {
    echo "Success: f no context!\n";
  }
}

<<__NoContext>>
function g() {
  try {
    ClassContext::getContext();
  } catch (TypeAssertionException $_) {
    echo "Success: g no context!\n";
  }
}

<<__EntryPoint>>
function main() {
  HH\without_implicit_context(fun('f'));
  ClassContext::start(new C, () ==> {
    HH\without_implicit_context(fun('f'));
  });
  HH\without_implicit_context(fun('g'));
  try {
    ClassContext::start(new C, fun('g'));
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  ClassContext::start(new C, () ==> {
    HH\without_implicit_context(fun('g'));
  });
}
