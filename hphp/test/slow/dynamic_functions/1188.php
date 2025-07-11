<?hh
<<__EntryPoint>>
function entrypoint_1188(): void {

  if (\HH\global_get('argc') > 100) {
    $f = 'var_dump';
  }
  else {
    $f = 'sscanf';
  }
  $auth = "24\tLewis Carroll";
  list($id, $first, $last) = HH\dynamic_fun($f)($auth, "%d\t%s %s");
  echo "$id,$first,$last\n";
}
