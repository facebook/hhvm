<?hh

function throwExn(): mixed {
  throw new Exception();
}

function printMSIC(): void {
  $c = MemoSensitiveIntCtx::getContext();
  echo($c ? $c->getPayload(): 'NULL')."\n";
}

function printMAIC()[leak_safe]: void {
  echo MemoAgnosticIntCtx::getContext()."\n";
}


<<__EntryPoint>>
function main(): mixed{
  include 'both-contexts.inc';
  try {
    MemoSensitiveIntCtx::start(new Base(1), () ==> {
      try {
        MemoAgnosticIntCtx::start(2, () ==> {
          printMAIC(); // should return 2
          printMSIC(); // should return 1
          throwExn();
        });
      } catch (Exception $e) {
        var_dump('caught!');
        printMAIC(); // should return NULL
        printMSIC(); // should still return 1
        throwExn();
      }
    });
  } catch (Exception $e) {
    var_dump('caught outer!');
    printMAIC(); // should return NULL
    printMSIC(); // should throw
  }

  try {
    MemoAgnosticIntCtx::start(1, () ==> {
    try {
      MemoSensitiveIntCtx::start(new Base(2), () ==> {
        printMAIC(); // should return 1, set on the outer
        printMSIC(); // should return 2
        throwExn();
      });
    } catch (Exception $e) {
      var_dump('caught!');
      printMAIC(); // should return 1 still
      printMSIC(); // should throw
      throwExn();
    }
  });
  } catch (Exception $e) {
    var_dump('caught outer!');
    printMAIC(); // should return NULL
    printMSIC(); // should throw
  }
}
