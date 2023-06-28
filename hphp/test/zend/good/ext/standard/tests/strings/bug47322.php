<?hh
function run($str, $fmt) :mixed{
  $r = sscanf($str, $fmt);
  return tuple($r[0] ?? null, $r[1] ?? null, $r[2] ?? null);
}

<<__EntryPoint>>
function main() :mixed{
  list($a, $b, $c) = run(":59:58","%s:%d:%f");
  $a__str = (string)($a);
  $b__str = (string)($b);
  $c__str = (string)($c);
  echo "[$a__str][$b__str][$c__str]\n";

  list($a, $b, $c) = run("15:01:58.2","%d:%f:%f");
  $a__str = (string)($a);
  $b__str = (string)($b);
  $c__str = (string)($c);
  echo "[$a__str][$b__str][$c__str]\n";

  list($a, $b, $c) = run("15.1111::foo","%f:%d:%s");
  $a__str = (string)($a);
  $b__str = (string)($b);
  $c__str = (string)($c);
  echo "[$a__str][$b__str][$c__str]\n";
}
