<?hh

final class A extends ClassFromHhi1 {} // `__NoAutoDynamic`-able

final class B extends ClassFromHhi2 {} // `__NoAutoDynamic`-able

function main(dynamic $dyn): void { // `__NoAutoDynamic`-able
  ClassFromHhi1::meth($dyn);
}
