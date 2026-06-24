<?hh

class C {
  public int $x = 1;
  public ?C $child = null;
}

// Constant (non-laundered) property names are still dynamic accesses (MPL) and
// are logged in every mode; HHBBC never folds dynamic prop access to static.
<<__EntryPoint>>
function main(): void {
  set_error_handler((int $errno, string $errstr) ==> {
    echo "notice: ".$errstr."\n";
    return true;
  });

  $o = new C();
  $o->child = new C();

  $px = 'x';
  $pc = 'child';

  echo $o->$px, "\n";                   // Get
  $o->$px = 10;                         // Set
  echo $o->$px, "\n";
  echo (isset($o->$px) ? 1 : 0), "\n";  // Isset
  echo $o->$pc->$px, "\n";              // Dim then Get
}
