<?hh // strict

namespace NS_Exception_class;

function displayExceptionObject(\Exception $e): void {
  echo "\$e = >$e<\n";		// calls __toString
  echo "getMessage:       >".$e->getMessage()."<\n";
  echo "getCode:          >".$e->getCode()."<\n";
  echo "getPrevious:      >".$e->getPrevious()."<\n";
  echo "getFile:          >".$e->getFile()."<\n";
  echo "getLine:          >".$e->getLine()."<\n";
  echo "getTraceAsString: >".$e->getTraceAsString()."<\n";

  $traceInfo = $e->getTrace();
  var_dump($traceInfo);
  echo "Trace Info:".((count($traceInfo) == 0) ? " none\n" : "\n");

  foreach ($traceInfo as $traceInfoKey => $traceLevel) {	// process all traceback levels
    echo "Key[$traceInfoKey]:\n";
    foreach ($traceLevel as $levelKey => $levelVal) {		// process one traceback level
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

function fL1(int $p1 = -10): void {
  try {
    echo "fL1: In try-block\n";

//  throw new \Exception();
    throw new \Exception("fL1 Message", 123);
  }
  catch (\Exception $e) {
    echo "fL1: In catch-block\n";

    displayExceptionObject($e);
  }
  finally {
    echo "fL1: In finally-block\n";
  }

  echo "fL1: Beyond try/catch/finally blocks\n==========\n";
  echo "fL1: Calling fL2\n";
  $a = -4.5;
  fL2(2.3, $a);	// pass 2nd arg as a non-literal to see how traceback handles it
//  fL2(2.3);	// see what happens when a default argument value is used
}

function fL2(float $p1, float $p2 = -100.0): void {
  try {
    echo "fL2: In try-block\n";

//    throw new \Exception();
    throw new \Exception("fL2 Message", 234);
  }
  catch (\Exception $e) {
    echo "fL2: In catch-block\n";

    displayExceptionObject($e);
  }
  finally {
    echo "fL2: In finally-block\n";
  }

  echo "fL2: Beyond try/catch/finally blocks\n==========\n";
  echo "fL2: Calling fL3\n";
  $a = "xyz"; $b = null; $c = true;
  fL3($a, $b, $c); // pass args as non-literals to see how traceback handles them
//  fL3($a, $b);	// see what happens when a default argument value is used
}

function fL3(string $p1, ?int $p2, bool $p3 = false): void {
  try {
    echo "fL3: In try-block\n";

//   throw new \Exception();
    throw new \Exception("fL3 Message", 345);
  }
  catch (\Exception $e) {
    echo "fL3: In catch-block\n";

    displayExceptionObject($e);
  }
  finally {
    echo "fL3: In finally-block\n";
  }

  echo "fL3: Beyond try/catch/finally blocks\n==========\n";
}

function main(): void {
  try {
    echo "L0: In try-block\n";

//    throw new \Exception();
    throw new \Exception("L0 Message", -1);
  }
  catch (\Exception $e) {
    echo "L0: In catch-block\n";

    displayExceptionObject($e);
  }
  finally {
    echo "L0: In finally-block\n";
  }

  echo "L0: Beyond try/catch/finally blocks\n==========\n";
  echo "L0: Calling fL1\n";
  fL1(10);
//  fL1();	// see what happens when a default argument value is used
}

/* HH_FIXME[1002] call to main in strict*/
main();
