<?hh
<<__EntryPoint>>
function entrypoint_1188(): void {

  if ($GLOBALS['argc'] > 100) {
    $f = 'var_dump';
  }
  else {
    $f = 'sscanf';
  }
  $auth = "24\tLewis Carroll";
  list($id, $first, $last) = $f($auth, "%d\t%s %s");
  echo "$id,$first,$last\n";
}
