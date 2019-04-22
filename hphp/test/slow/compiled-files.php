<?hh

<<__EntryPoint>>
function main() {
  try {
    $f = tempnam(sys_get_temp_dir(), 'compile_me');
    file_put_contents($f, "<?hh echo \"hello $f!\\n\";");
    include $f;
    var_dump(array_key_exists($f, hh\get_compiled_units()));
  } finally {
    if (isset($f)) unlink($f);
  }
}
