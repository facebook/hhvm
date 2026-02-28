<?hh

class Testing86Pinit {
  private $x = Bar::A;

  public function asd() :mixed{
    if ($this->x) {
      echo "this should happen\n";
    } else {
      echo "hmm\n";
    }
  }
}

function main() :mixed{
  $x = new Testing86Pinit();
  $x->asd();
}


/*
 * Testing 86pinit's effects on private property type inference.
 */

<<__EntryPoint>>
function main_private_props_006() :mixed{
if (mt_rand()) {
  include 'private_props_006-1.inc';
} else {
  include 'private_props_006-2.inc';
}

main();
}
