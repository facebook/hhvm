<?hh

type Tany<T> = T;

function f<reify T>(<<__Soft>> T $t): void {
  echo "\$t: $t [reified]\n";
}
function f_tany(<<__Soft>> Tany $tany): void {
  echo "\$tany: $tany\n";
}
class Typeconsts {
  const type Tany = Tany;
}

<<__EntryPoint>>
function main(): void {
  printf("\n===== Tany =====\n");
  f_tany(4);
  f_tany("string");
  f<Tany>(4);
  f<Tany>("string");

  printf("\n===== Typeconsts::Tany =====\n");
  f<Typeconsts::Tany>(4);
  f<Typeconsts::Tany>("string [typeconst]");

  $rt = new ReflectionClass(Typeconsts::class)->getTypeConstant("Tany");
  printf("Type structure kind: %d\n", $rt->getTypeStructure()["kind"]);
  printf("Assigned type text: %s\n", $rt->getAssignedTypeText() ?? "<missing>");
}
