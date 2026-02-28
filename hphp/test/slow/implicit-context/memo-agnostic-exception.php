<?hh

function throwExn(): void{
  throw new Exception();
}

<<__EntryPoint>>
function main(): mixed{
  include 'memo-agnostic.inc';

  TestContext::start(1, () ==> {
    try {
      TestContext::start(2, () ==> {
        var_dump(TestContext::getContext());
        throwExn();
      });
    } catch (Exception $e) {
      var_dump('caught!');
      var_dump(TestContext::getContext());
    }
  });
}
