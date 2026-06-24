<?hh

class C {
  public int $x = 1;
  public ?C $child = null;
}

// Exercises every dynamic-property-access op kind with the notice option set.
// The error handler captures each notice so the expected output is path-free.
<<__EntryPoint>>
function main(): void {
  set_error_handler((int $errno, string $errstr) ==> {
    echo "notice: ".$errstr."\n";
    return true;
  });

  $o = new C();
  $o->child = new C();

  $px = __hhvm_intrinsics\launder_value('x');
  $pc = __hhvm_intrinsics\launder_value('child');

  echo $o->$px, "\n";                       // Get
  $o->$px = 10;                             // Set
  echo $o->$px, "\n";
  echo (isset($o->$px) ? 1 : 0), "\n";      // Isset
  $o->$px++;                                // IncDec
  echo $o->$px, "\n";
  $o->$px += 5;                             // SetOp
  echo $o->$px, "\n";
  echo $o->$pc->$px, "\n";                  // Dim ($o->$pc) then Get (->$px)
}
