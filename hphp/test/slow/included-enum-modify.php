<?hh

<<__EntryPoint>>
function main() :mixed{
  $b = null;
  $val = apc_fetch('val', inout $b) ?: 42;
  apc_store('val', $val + 1);

  try {
    $iefile = apc_fetch('iefile', inout $b) ?: tempnam(sys_get_temp_dir(), 'included_enum');
    apc_store('iefile', $iefile);

    // If we write a file multiple times really fast then we can modify the
    // file during the same time tick as last time and the change won't be
    // noticed. Unfortunate but difficult to fix (we'd need an inotify handler
    // which is probably overkill).  Thus - the sleep call in the following
    // code.  It would probably be reasonable to usleep() but some filesystems
    // won't support subsecond modification times.
    sleep(1);
    file_put_contents($iefile, "<?hh enum BarEnum : int { VAL = $val; }");
    include $iefile;
    include __FILE__.'.inc';

    var_dump(FooEnum::getValues());
  } finally {
    if ($iefile) unlink($iefile);
  }
}
