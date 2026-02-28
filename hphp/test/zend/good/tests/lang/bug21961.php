<?hh

class man {
  public $name, $bars;

  function __construct() {
    $this->name = 'Mr. X';
    $this->bars = vec[];
  }

  function getdrunk($where) :mixed{
    $this->bars[] = new bar($where);
  }

  function getName() :mixed{
    return $this->name;
  }
}

class bar extends man {
  public $name;

  function __construct($w) {
    $this->name = $w;
  }

  function getName() :mixed{
    return $this->name;
  }

  function whosdrunk() :mixed{
    $who = get_parent_class($this);
    if($who == NULL) {
      return 'nobody';
    }
    return man::getName();
  }
}

<<__EntryPoint>>
function main() :mixed{
  $x = new man;
  $x->getdrunk('The old Tavern');
  var_dump($x->bars[0]->whosdrunk());
}
