<?hh

function f<reify T>(<<__Soft>> T $t): void {
  echo "\$t: $t [reified]\n";
}
function f_supportdyn(<<__Soft>> HH\FIXME\SUPPORTDYN_MARKER<int> $supportdyn): void {
  echo "\$supportdyn: $supportdyn\n";
}

class Typeconsts {
  const type Tsupportdyn = HH\FIXME\SUPPORTDYN_MARKER<int>;
}

<<__EntryPoint>>
function main(): void {
  printf("\n===== HH\FIXME\SUPPORTDYN_MARKER<int> =====\n");
  f_supportdyn(4);
  f_supportdyn("not supportdyn int");
  f<HH\FIXME\SUPPORTDYN_MARKER<int>>(4);
  f<HH\FIXME\SUPPORTDYN_MARKER<int>>("not supportdyn int");

  printf("\n===== Typeconsts::Tsupportdyn =====\n");
  f<Typeconsts::Tsupportdyn>(4);
  f<Typeconsts::Tsupportdyn>("not supportdyn int [typeconst]");

  $rt = new ReflectionClass(Typeconsts::class)->getTypeConstant("Tsupportdyn");
  printf("Type structure kind: %d\n", $rt->getTypeStructure()["kind"]);
  printf("Assigned type text: %s\n", $rt->getAssignedTypeText() ?? "<missing>");
}
