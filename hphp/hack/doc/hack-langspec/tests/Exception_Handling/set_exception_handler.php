<?hh // strict

namespace NS_set_exception_handler;

function displayExceptionObject(\Exception $e): void {
  echo "\$e = >$e<\n";	// calls __toString
  echo "getMessage:       >".$e->getMessage()."<\n";
  echo "getCode:          >".$e->getCode()."<\n";
  echo "getPrevious:      >".$e->getPrevious()."<\n";
  echo "getFile:          >".$e->getFile()."<\n";
  echo "getLine:          >".$e->getLine()."<\n";
  echo "getTraceAsString: >".$e->getTraceAsString()."<\n";

  $traceInfo = $e->getTrace();
  var_dump($traceInfo);
  echo "Trace Info:".((count($traceInfo) == 0) ? " none\n" : "\n");
  foreach ($traceInfo as $traceInfoKey => $traceLevel) { // process all traceback levels
    echo "Key[$traceInfoKey]:\n";
    foreach ($traceLevel as $levelKey => $levelVal) {	// process one traceback level
      if ($levelKey != "args") {
        echo "  Key[$levelKey] => >$levelVal<\n";
      } else {
        echo "  Key[$levelKey]:\n";
        foreach ($levelVal as $argKey => $argVal) {	// process all args for that level
          echo "    Key[$argKey] => >$argVal<\n";
        }
      }
    }
  }
}

function MyDefExHandler(\Exception $e): void {
  echo "In MyDefExHandler\n";
  displayExceptionObject($e);
  echo "Leaving MyDefExHandler\n";
}

function f(int $p1, bool $p2): void {
  try {
    echo "In f's try-block\n";

    throw new \Exception("Watson, come here!", 1234);
  }

// no catch block(s)

  finally {
    echo "In f's finally-block\n";
  }

  echo "Beyond try/catch/finally blocks\n==========\n";
}

function main(): void {
// define a default un-caught exception handler

  $prev = set_exception_handler(null);	// set to default handler
  var_dump($prev);

// establish a new un-caught exception handler

  $prev = set_exception_handler('\NS_set_exception_handler\MyDefExHandler');	// use my handler
  var_dump($prev);

//  restore_exception_handler();

  echo "About to call f\n";
  f(10, true);
  echo "Beyond the call to f()\n";	// never gets here; script terminates after my handler ends
}

/* HH_FIXME[1002] call to main in strict*/
main();
