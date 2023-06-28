<?hh

function throwExn()[zoned] :mixed{
  throw new Exception();
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';

  IntContext::start(1, ()[zoned] ==> {
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
