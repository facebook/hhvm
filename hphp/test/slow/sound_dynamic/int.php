<?hh

function f<reify T>(<<__Soft>> T $t): void {
  echo "\$t: $t [reified]\n";
}
function f_int(<<__Soft>> int $i): void {
  echo "\$i: $i\n";
}
class Typeconsts {
  const type Tint = int;
}

<<__EntryPoint>>
function main(): void {
  printf("\n===== int =====\n");
  f_int(4);
  f_int("not int");
  f<int>(4);
  f<int>("not int");

  printf("\n===== Typeconsts::Tint =====\n");
  f<Typeconsts::Tint>(4);
  f<Typeconsts::Tint>("not int [typeconst]");

  $rt = new ReflectionClass(Typeconsts::class)->getTypeConstant("Tint");
  printf("Type structure kind: %d\n", $rt->getTypeStructure()["kind"]);
  printf("Assigned type text: %s\n", $rt->getAssignedTypeText() ?? "<missing>");
}
