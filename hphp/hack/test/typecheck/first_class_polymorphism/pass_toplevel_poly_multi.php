<?hh

class Biggest {}
class Big extends Biggest {}
class Small extends Big {}
class Smaller extends Small{}
class Smallest extends Smaller {}

function multi<T1 as T2 super T3, T2 as Biggest, T3 as Smallest, T4 as T1>(
  T1 $_,
  T2 $_,
  T4 $_
): ?T3 {
  return null;
}

function rcvr1((function(Small,Big,Smallest): ?Smallest) $_): void {}
function rcvr2((function(Small,Biggest,Smaller): ?Smallest) $_): void {}

function pass_generic(): void {
  $f = multi<>;
  rcvr1($f);
  rcvr2($f);
}
