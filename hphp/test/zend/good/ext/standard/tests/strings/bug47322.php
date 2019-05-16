<?hh
function run($str, $fmt) {
  $r = sscanf($str, $fmt);
  return tuple($r[0] ?? null, $r[1] ?? null, $r[2] ?? null);
}

<<__EntryPoint>>
function main() {
  list($a, $b, $c) = run(":59:58","%s:%d:%f");
  echo "[$a][$b][$c]\n";

  list($a, $b, $c) = run("15:01:58.2","%d:%f:%f");
  echo "[$a][$b][$c]\n";

  list($a, $b, $c) = run("15.1111::foo","%f:%d:%s");
  echo "[$a][$b][$c]\n";
}
