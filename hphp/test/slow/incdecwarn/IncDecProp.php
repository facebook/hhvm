<?hh

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
  var_dump(++$o->p);
  var_dump(--$o->q);
  var_dump($o->r++);
  var_dump($o->s--);
  print_r($o);

  print "Test end\n";
}
