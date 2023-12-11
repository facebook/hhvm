<?hh

class Small {
  private static $nc = 0;
  public $name;
  public $num;
  function __construct() {
    $n = self::$nc++;
    $this->name = 'foo'.$n;
    $this->num = 3*$n;
  }
}
class Big {
  public $groupAll = vec[];
  public $group1 = vec[];
  public $group2 = vec[];
  public $wacky;
  public $nothing;
  public $unrelated = vec[];
  function add() :mixed{
    $s = new Small();
    $this->groupAll[] = $s;
    if ($s->num % 2 == 0) {
      $this->group1[]=vec[$s->name, $s];
    }
 else {
      $this->group2[]=vec[$s->name, $s];
    }
  }
  function finish() :mixed{
    $x = 10;
    $this->wacky = vec[$x, $x];
    $s = new Small();
    $this->unrelated[] = $s;
    $this->unrelated[] = $s;
    $this->unrelated[] = $s;
  }
}
function t() :mixed{
  $b = new Big;
  for ($i = 0;
 $i < 10;
 ++$i) {
    $b->add();
  }
  $b->finish();
  var_dump($b);
  $s = serialize($b);
  var_dump($s);
  $us = unserialize($s);
  var_dump($us);
}

<<__EntryPoint>>
function main_1541() :mixed{
t();
}
