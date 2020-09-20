<?hh

class C {
  public $preInc = 0;
  public $preDec = 0;
  public $postInc = 0;
  public $postDec = 0;
}

function error_boundary(inout $x, $fn) {
  try {
    $fn(inout $x);
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
  }
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
  var_dump(++$o->p);
  var_dump(--$o->q);
  var_dump($o->r++);
  var_dump($o->s--);
  print_r($o);

  print "--- null ---\n";
  $o = null;
  error_boundary(inout $o, (inout $o) ==> ++$o->preInc);
  error_boundary(inout $o, (inout $o) ==> --$o->preDec);
  error_boundary(inout $o, (inout $o) ==> $o->postInc++);
  error_boundary(inout $o, (inout $o) ==> $o->postDec--);
  var_dump($o);

  print "--- 42 ---\n";
  $o = 42;
  var_dump(++$o->preInc);
  print_r($o);
  print "\n";

  print "Test end\n";
}
