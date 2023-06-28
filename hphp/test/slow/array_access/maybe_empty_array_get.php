<?hh

class Gronk {
  private $gronks;

  function __construct() {
    $this->gronks = varray[new stdClass, new stdClass, new stdClass];
  }

  public function gronkify(): void {
    try {
      list($stuff, $gronk) = $this->getStuffAndGronk(null);
      var_dump($stuff);
      var_dump($gronk);
    } catch (Exception $e) { echo $e->getMessage()."\n"; }
  }

  private function getStuffAndGronk(?int $count): varray {
    foreach ($this->gronks as $gronk) {
      if (\HH\global_get('break')) {
        $z = varray[varray[null, null], $gronk];
        $z[] = new stdClass;
        return $z;
      }
    }
    return varray[];
  }
}


<<__EntryPoint>>
function main_maybe_empty_array_get() :mixed{
$y = new Gronk;

$break = false;
for ($i = 0; $i < 3; ++$i) $y->gronkify();
}
