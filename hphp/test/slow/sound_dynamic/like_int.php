<?hh
<<file:__EnableUnstableFeatures('like_type_hints')>>

function f<reify T>(<<__Soft>> T $t): void {
  echo "\$t: $t [reified]\n";
}
function f_like(<<__Soft>> ~int $li): void {
  echo "\$li: $li\n";
}
class Typeconsts {
  const type Tlike = ~int;
}

<<__EntryPoint>>
function main(): void {
  printf("\n===== ~int =====\n");
  f_like(4);
  f_like("not like int");
  f<~int>(4);
  f<~int>("not like int");

  printf("\n===== Typeconsts::Tlike =====\n");
  f<Typeconsts::Tlike>(4);
  f<Typeconsts::Tlike>("not like int [typeconst]");

  $rt = new ReflectionClass(Typeconsts::class)->getTypeConstant("Tlike");
  printf("Type structure kind: %d\n", $rt->getTypeStructure()["kind"]);
  printf("Assigned type text: %s\n", $rt->getAssignedTypeText() ?? "<missing>");
}
