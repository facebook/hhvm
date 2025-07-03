<?hh

<<file:__EnableUnstableFeatures('require_class', 'require_constraint')>>

class E {}

class C extends E {}

final class D extends C {}

interface I {}

trait T {
  require extends C;
  require class D;
  require implements I;
  require this as E;
}

<<__EntryPoint>>
function main() : void {
  $rc = new ReflectionClass("T");

  echo " -- getRequirementNames --\n";
  foreach ($rc->getRequirementNames() as $req) {
    print $req . "\n";
  }

  echo " -- getRequiredClass --\n";
  print ($rc->getRequiredClass()) . "\n";
}
