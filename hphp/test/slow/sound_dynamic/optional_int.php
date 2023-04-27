<?hh

function f<reify T>(<<__Soft>> T $t): void {
  echo "\$t: $t [reified]\n";
}
function f_nint(<<__Soft>> ?int $ni): void {
  echo "\$ni: $ni\n";
}

class Typeconsts {
  const type Tnint = ?int;
}

<<__EntryPoint>>
function main(): void {
  printf("\n===== ?int =====\n");
  f_nint(4);
  f_nint("not optional int");
  f<?int>(4);
  f<?int>("not optional int");

  printf("\n===== Typeconsts::Tnint =====\n");
  f<Typeconsts::Tnint>(4);
  f<Typeconsts::Tnint>("not optional int [typeconst]");

  $rt = new ReflectionClass(Typeconsts::class)->getTypeConstant("Tnint");
  printf("Type structure kind: %d\n", $rt->getTypeStructure()["kind"]);
  printf("Assigned type text: %s\n", $rt->getAssignedTypeText() ?? "<missing>");
}
