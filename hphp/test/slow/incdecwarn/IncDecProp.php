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
  var_dump(++$o->preInc);
  var_dump(--$o->preDec);
  var_dump($o->postInc++);
  var_dump($o->postDec--);
  error_boundary(inout $o, (inout $o) ==> var_dump(++$o->p));
  error_boundary(inout $o, (inout $o) ==> var_dump(--$o->q));
  error_boundary(inout $o, (inout $o) ==> var_dump($o->r++));
  error_boundary(inout $o, (inout $o) ==> var_dump($o->s--));
  var_dump($o);

  print "Test end\n";
}
