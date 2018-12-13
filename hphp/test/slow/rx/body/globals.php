<?hh

<<__Rx>>
function test() {
  $x = $GLOBALS['_GET'];            // CGetG
  $x = $GLOBALS['WAT'] ?? false;    // CGetQuietG
  $x = isset($GLOBALS['WAT']);      // IssetG
  $x = empty($GLOBALS['WAT']);      // EmptyG

  $GLOBALS['WAT'] = 1;              // SetG
  $GLOBALS['WAT'] *= 2;             // SetOpG
  $GLOBALS['WAT']++;                // IncDecG
  unset($GLOBALS['WAT']);           // UnsetG

  $x = $GLOBALS['GLOBALS']['_GET']; // BaseGC
  $l = 'GLOBALS';
  $x = $GLOBALS[$l]['_GET'];        // BaseGL
}

<<__EntryPoint>>
function main() {
  test();
  echo "Done\n";
}
