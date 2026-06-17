<?hh

/**
 * Native Facts is enabled (Autoload.Query is set) but Autoload.DB resolves to an
 * empty path. Previously getDBKey() hit always_assert(!dbPath.empty()) and
 * aborted the entire HHVM process (SIGABRT), taking down the webserver. Now it
 * throws a catchable, explanatory error which HH\Facts\db_path() surfaces as an
 * InvalidOperationException, so the request survives and the message explains
 * exactly what is wrong (rather than a generic "rebuild the DB" suggestion).
 */
<<__EntryPoint>>
function main(): void {
  print "before\n";
  try {
    HH\Facts\db_path("");
    print "FAIL: expected HH\\Facts\\db_path() to throw\n";
  } catch (InvalidOperationException $e) {
    print "caught: ".$e->getMessage()."\n";
  }
  print "after\n";
}
