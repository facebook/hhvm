<?hh

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
  include 'implicit.inc';

  HH\without_implicit_context(f<>);
  ClassContext::start(new C, () ==> {
    HH\without_implicit_context(f<>);
  });
  HH\without_implicit_context(g<>);
  try {
    ClassContext::start(new C, g<>);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  ClassContext::start(new C, () ==> {
    HH\without_implicit_context(g<>);
  });
}
