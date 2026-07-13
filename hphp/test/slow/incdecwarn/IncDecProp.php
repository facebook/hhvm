<?hh

function error_boundary(inout $x, $fn) :mixed{
  try {
    $fn(inout $x);
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
  }
}

class C {
  public $preInc = 0;
  public $preDec = 0;
  public $postInc = 0;
  public $postDec = 0;
}

<<__EntryPoint>>
function main_entry(): void {

  print "Test begin\n";

  print "--- C ---\n";
  $o = new C;
  ++$o->preInc; var_dump($o->preInc);
  --$o->preDec; var_dump($o->preDec);
  $t = $o->postInc; $o->postInc++; var_dump($t);
  $t = $o->postDec; $o->postDec--; var_dump($t);
  error_boundary(inout $o, (inout $o) ==> { ++$o->p; var_dump($o->p); });
  error_boundary(inout $o, (inout $o) ==> { --$o->q; var_dump($o->q); });
  error_boundary(inout $o, (inout $o) ==> { $o->r++; var_dump($o->r); });
  error_boundary(inout $o, (inout $o) ==> { $o->s--; var_dump($o->s); });
  var_dump($o);

  print "Test end\n";
}
