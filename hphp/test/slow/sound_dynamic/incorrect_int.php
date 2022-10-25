<?hh

function f<reify T>(<<__Soft>> T $t): void {
  echo "\$t: $t [reified]\n";
}
function f_incorrect(<<__Soft>> HH\INCORRECT_TYPE<int> $ii): void {
  echo "\$ii: $ii\n";
}

class Typeconsts {
  const type Tincorrect = HH\INCORRECT_TYPE<int>;
}

<<__EntryPoint>>
function main(): void {
  printf("\n===== HH\INCORRECT_TYPE<int> =====\n");
  f_incorrect(4);
  f_incorrect("not incorrect int");
  f<HH\INCORRECT_TYPE<int>>(4);
  f<HH\INCORRECT_TYPE<int>>("not incorrect int");

  printf("\n===== Typeconsts::Tincorrect =====\n");
  f<Typeconsts::Tincorrect>(4);
  f<Typeconsts::Tincorrect>("not incorrect int [typeconst]");

  $rt = new ReflectionClass(Typeconsts::class)->getTypeConstant("Tincorrect");
  printf("Type structure kind: %d\n", $rt->getTypeStructure()["kind"]);
  printf("Assigned type text: %s\n", $rt->getAssignedTypeText() ?? "<missing>");
}
