<?hh

function captureUninit() :mixed{
  echo "\n== captureUninit ==\n\n";

  $foo = () ==> { // no warning for $x
    $x = 2;
    var_dump($x);
  };

  $foo(); // 2

  try {
    var_dump($x); // uninit
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }

  $x = 1;
  var_dump($x); // 1
}

function captureCatchUninit() :mixed{
  echo "\n== captureCatchUninit ==\n\n";

  try {
    $foo = () ==> { // no warning for $e
      try {
        throw Exception('...');
      } catch (Exception $e) {
        var_dump($e);
      }
    };
  } catch (Exception $e) {
  }

  try {
    var_dump($e); // uninit
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

function captureUninitWarning() :mixed{
  echo "\n== captureUninitWarning ==\n\n";

  $foo = () ==> { // no warning for $x
    try {
      var_dump($x); // uninit
    } catch (UndefinedVariableException $e) {
      var_dump($e->getMessage());
    }
  };

  try {
    var_dump($x); // uninit
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }

  $x = 1;
  var_dump($x); // 1

  $foo(); // still warning for $x
}

function captureExplicitUninit() :mixed{
  echo "\n== captureExplicitUninit ==\n\n";

  try {
    $foo = function() use ($x) { // fatal here for $x
      $x = 2;
      var_dump($x);
    };

    $foo(); // 2
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}


<<__EntryPoint>>
function main_lambda_uninit_capture() :mixed{
  captureUninit();
  captureCatchUninit();
  captureUninitWarning();
  captureExplicitUninit();
}
