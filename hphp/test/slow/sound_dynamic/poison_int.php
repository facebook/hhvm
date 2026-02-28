<?hh

function f<reify T>(<<__Soft>> T $t): void {
  echo "\$t: $t [reified]\n";
}
function f_poison(<<__Soft>> HH\FIXME\POISON_MARKER<int> $poison): void {
  echo "\$poison: $poison\n";
}

class Typeconsts {
  const type Tpoison = HH\FIXME\POISON_MARKER<int>;
}

<<__EntryPoint>>
function main(): void {
  printf("\n===== HH\FIXME\POISON_MARKER<int> =====\n");
  f_poison(4);
  f_poison("not poison int");
  f<HH\FIXME\POISON_MARKER<int>>(4);
  f<HH\FIXME\POISON_MARKER<int>>("not poison int");

  printf("\n===== Typeconsts::Tpoison =====\n");
  f<Typeconsts::Tpoison>(4);
  f<Typeconsts::Tpoison>("not poison int [typeconst]");

  $rt = new ReflectionClass(Typeconsts::class)->getTypeConstant("Tpoison");
  printf("Type structure kind: %d\n", $rt->getTypeStructure()["kind"]);
  printf("Assigned type text: %s\n", $rt->getAssignedTypeText() ?? "<missing>");
}
