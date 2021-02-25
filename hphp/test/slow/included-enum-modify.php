<?hh

<<__EntryPoint>>
function main() {
  $b = null;
  $val = apc_fetch('val', inout $b) ?: 42;
  apc_store('val', $val + 1);

  try {
    $iefile = apc_fetch('iefile', inout $b) ?: tempnam(sys_get_temp_dir(), 'included_enum');
    apc_store('iefile', $iefile);

    file_put_contents($iefile, "<?hh enum BarEnum : int { VAL = $val; }");
    include $iefile;
    include __FILE__.'.inc';

    var_dump(FooEnum::getValues());
  } finally {
    if ($iefile) unlink($iefile); 
  }
}
