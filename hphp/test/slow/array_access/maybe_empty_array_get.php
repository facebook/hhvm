<?hh

class Gronk {
  private $gronks;

  function __construct() {
    $this->gronks = array(new stdclass, new stdclass, new stdclass);
  }

  public function gronkify(): void {
    list($stuff, $gronk) = $this->getStuffAndGronk(
      null
    );

    var_dump($stuff);
    var_dump($gronk);
  }

  private function getStuffAndGronk(?int $count): array {
    foreach ($this->gronks as $gronk) {
      if ($GLOBALS['break']) {
        $z = array(array(null, null), $gronk);
        $z[] = new stdclass;
        return $z;
      }
    }
    return array();
  }
}

$y = new Gronk;

$break = false;
for ($i = 0; $i < 3; ++$i) $y->gronkify();
