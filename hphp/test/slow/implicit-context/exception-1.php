<?hh

function throwExn()[zoned] :mixed{
  throw new Exception();
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';

  ClassContext::start(new Base(1), ()[zoned] ==> {
    try {
      ClassContext::start(new Base(2), () ==> {
        var_dump(ClassContext::getContext()->getPayload());
        throwExn();
      });
    } catch (Exception $e) {
      var_dump('caught!');
      var_dump(ClassContext::getContext()->getPayload());
    }
  });
}
