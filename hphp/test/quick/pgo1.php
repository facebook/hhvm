<?hh


class Repro {
  private $propStr;
  private $propInt = 5;

  public function __construct($pn) {
    $this->propStr = $pn;
  }

  final public function getPropStr() {
    return $this->propStr;
  }

  private static $reproStaticarr = array();

  final public function repro($user_id) {
    $key = $this->getPropStr() . (string)$this->propInt;
    if (!(isset(self::$reproStaticarr[$key]))) {
      self::$reproStaticarr[$key] = array();
    }
  }
}

$rd = new Repro("proj");

foreach (range(1,1000) as $i) {
  $rd->repro(0);
}

var_dump($rd);
