<?hh

<<__EntryPoint>>
function main() {
  try {
    $f = tempnam(sys_get_temp_dir(), 'compile_me');
    file_put_contents($f, "<?hh function hello() { echo \"hello $f!\\n\"; }");
    include $f;
    hello();
    var_dump(array_key_exists($f, hh\get_compiled_units()));
  } finally {
    if (isset($f)) unlink($f);
  }
}
