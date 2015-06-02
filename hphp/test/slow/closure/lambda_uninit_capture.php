<?hh

function captureUninit() {
  echo "\n== captureUninit ==\n\n";

  $foo = () ==> { // no warning for $x
    $x = 2;
    var_dump($x);
  };

  $foo(); // 2

  var_dump($x); // uninit

  $x = 1;
  var_dump($x); // 1
}

function captureCatchUninit() {
  echo "\n== captureCatchUninit ==\n\n";

  try {
    $foo = () ==> { // no warning for $e
      try {
        throw Exception('...');
      } catch (Exception $e) {
        var_dump($e);
      }
    };
  } catch (Exception $e) {}

  var_dump($e); // uninit
}

function captureUninitWarning() {
  echo "\n== captureUninitWarning ==\n\n";

  $foo = () ==> { // no warning for $x
    var_dump($x);
  };

  var_dump($x); // uninit

  $x = 1;
  var_dump($x); // 1

  $foo(); // still warning for $x
}

function captureExplicitUninit() {
  echo "\n== captureExplicitUninit ==\n\n";

  $foo = function() use ($x) { // warning here for $x
    $x = 2;
    var_dump($x);
  };

  $foo(); // 2

  var_dump($x); // uninit

  $x = 1;
  var_dump($x); // 1
}

captureUninit();
captureCatchUninit();
captureUninitWarning();
captureExplicitUninit();
