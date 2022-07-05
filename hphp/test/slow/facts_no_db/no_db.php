<?hh

/**
 * Test what happens if `Autoload.DB.CanCreate=false`
 */
<<__EntryPoint>>
function no_db(): void {
  $enabled = HH\Facts\enabled() ? 'true' : 'false';
  print "HH\\Facts\\enabled() = $enabled\n";
}
