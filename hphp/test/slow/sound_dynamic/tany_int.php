<?hh

function f<reify T>(<<__Soft>> T $t): void {
  echo "\$t: $t [reified]\n";
}
function f_tany_int(<<__Soft>> HH\FIXME\TANY_MARKER<int> $tany_int): void {
  echo "\$tany_int: $tany_int\n";
}

class Typeconsts {
  const type Tany_int = HH\FIXME\TANY_MARKER<int>;
}

<<__EntryPoint>>
function main(): void {
  printf("\n===== HH\FIXME\TANY_MARKER<int> =====\n");
  f_tany_int(4);
  f_tany_int("not Tany int");
  f<HH\FIXME\TANY_MARKER<int>>(4);
  f<HH\FIXME\TANY_MARKER<int>>("not Tany int");

  printf("\n===== Typeconsts::Tany_int =====\n");
  f<Typeconsts::Tany_int>(4);
  f<Typeconsts::Tany_int>("not Tany int [typeconst]");

  $rt = new ReflectionClass(Typeconsts::class)->getTypeConstant("Tany_int");
  printf("Type structure kind: %d\n", $rt->getTypeStructure()["kind"]);
  printf("Assigned type text: %s\n", $rt->getAssignedTypeText() ?? "<missing>");
}
