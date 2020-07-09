<?hh

function throwExn() {
  throw new Exception();
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';

  IntContext::start(1, () ==> {
    try {
      IntContext::start(2, () ==> {
        var_dump(IntContext::getContext());
        throwExn();
      });
    } catch (Exception $e) {
      var_dump('caught!');
      var_dump(IntContext::getContext());
    }
  });
}
