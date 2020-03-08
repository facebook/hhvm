<?hh // strict

namespace NS_anonymous_function_name;

class C {
  public function f(): void {
    echo "__FUNCTION__ = " . __FUNCTION__ . "\n";
    echo "__METHOD__ = " . __METHOD__ . "\n";

    $res = (function (): void {		
      echo "__FUNCTION__ = " . __FUNCTION__ . "\n";
      echo "__METHOD__ = " . __METHOD__ . "\n";
    });

    $res();
  }
}

function main(): void {
  echo "__FUNCTION__ = " . __FUNCTION__ . "\n";
  echo "__METHOD__ = " . __METHOD__ . "\n";

  $res = (function (): void {
    echo "__FUNCTION__ = " . __FUNCTION__ . "\n";
    echo "__METHOD__ = " . __METHOD__ . "\n";
  });

  $res();

  $c = new C();
  $c->f();
}

/* HH_FIXME[1002] call to main in strict*/
main();
