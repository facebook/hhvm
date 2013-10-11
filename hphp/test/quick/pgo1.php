<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Repro {
  private $propStr;
  private $propInt = 5;

  public function __construct($pn) {
    $this->propStr = $pn;
  }

  final public function getPropStr() {
    return $this->propStr;
  }

  final public function repro($user_id) {
    static $staticArr = array();
    $key = $this->getPropStr() . (string)$this->propInt;
    if (!(isset($staticArr[$key]))) {
      $staticArr[$key] = array();
    }
  }
}

$rd = new Repro("proj");

foreach (range(1,1000) as $i) {
  $rd->repro(0);
}

var_dump($rd);
