<?hh

<<file:__EnableUnstableFeatures('require_class')>>

class C {}

final class D extends C {}

interface I {}

trait T {
  require extends C;
  require class D;
  require implements I;
}

<<__EntryPoint>>
function main() : void {
  $rc = new ReflectionClass("T");

  foreach ($rc->getRequirementNames() as $req) {
    print $req . "\n";
  }
}
