<?hh // strict

namespace NS_pre_increment_and_decrement;

class C {
  public int $prop1 = 0;
}

function main(): void {
  echo "=== int 5 ===\n";

  $v = 5; var_dump(++$v); var_dump(--$v);

  echo "\n=== int 0x7fffffffffffffff ===\n";

  $v = 0x7fffffffffffffff; var_dump(++$v); var_dump(--$v);

  echo "\n=== int 0x80000000 ===\n";

  $v = 0x8000000000000000; var_dump(++$v); var_dump(--$v);

  echo "\n=== float 12.345 ===\n";

  $v = 12.345; var_dump(++$v); var_dump(--$v);

  echo "\n=== float INF ===\n";

  $v = INF; var_dump(++$v); var_dump(--$v);

  echo "\n=== float -INF ===\n";

  $v = -INF; var_dump(++$v); var_dump(--$v);

  echo "\n=== float NAN ===\n";

  $v = NAN; var_dump(++$v); var_dump(--$v);
/*
// Hack does not permit null to be the object of a prefix ++/--

  echo "\n=== null ===\n";

  $v = null; var_dump(++$v); var_dump(--$v);
*/
/*
// Hack does not permit Boolean values to be the object of a prefix ++/--

  echo "\n=== true ===\n";

  $v = true; var_dump(++$v); var_dump(--$v);

  echo "\n=== false ===\n";

  $v = false; var_dump(++$v); var_dump(--$v);
*/
/*
// Hack does not permit string values to be the object of a prefix ++/--
  echo "\n=== string \"\" ===\n";

  echo "---> ++/--/--\n";
  $v = ""; var_dump(++$v); var_dump($v--); var_dump(--$v);
  echo "\n---> --/++/++\n";
  $v = ""; var_dump(--$v); var_dump(++$v); var_dump(++$v);

  echo "\n=== string \"0\" ===\n";

  echo "---> ++/--/--\n";
  $v = "0"; var_dump(++$v); var_dump(--$v); var_dump(--$v);
  echo "\n---> --/++/++\n";
  $v = "0"; var_dump(--$v); var_dump($v++); var_dump($v++);

  echo "\n=== string \"-26\" ===\n";

  echo "---> ++/--/--\n";
  $v = "-26"; var_dump(++$v); var_dump($v--); var_dump(--$v);
  echo "\n---> --/++/++\n";
  $v = "-26"; var_dump(--$v); var_dump($v++); var_dump(++$v);

  echo "\n=== string \"a\" ===\n";

  echo "---> ++/--/--\n";
  $v = "a"; var_dump(++$v); var_dump($v--); var_dump(--$v);
  echo "\n---> --/++/++\n";
  $v = "a"; var_dump(--$v); var_dump($v++); var_dump(++$v);
*/
 echo "=== ?int ===\n";

  $c = new C();

  echo "---> ++/--/--\n";
  $c->prop1 = 5; var_dump(++$c->prop1); var_dump(--$c->prop1); var_dump(--$c->prop1);
}

/* HH_FIXME[1002] call to main in strict*/
main();
