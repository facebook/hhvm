<?php

class KM {
  private $kSA;
  private $specs;

  function getKS() {
    return $this->kSA;
  }
  function __construct() {
    $this->specs = array(
      "r:3600" =>
        array('oblt' => 0,
          'ms' => 10000000,
          'ttdm' => 100)
      );
  }

  private static function decay($a, $b, $c, $d) { return 0; }

  private function getXXX() {
    return array(1, 0, 0);
  }
  public function getInfo() {
    $time = time();
    $this->kSA = array();
    foreach ($this->specs as $key => $spec) {
      $fetched = $this->getXXX();
      list($sc, $l_o, $l_b_t) =
        array(
          $fetched[0],
          $fetched[1],
          $fetched[2]
        );

      if ($time - $l_b_t > $spec['oblt']) {
        if ($sc > $spec['ms']) {
          $sc = 0.5 * $spec['ms'];
        } else {
          $sc = self::decay(
            $spec['ms'],
            $spec['ttdm'],
            $sc,
            $l_o
          );
        }
        $this->dKV[$key] = array(
          'sc' => (double)$sc,
          'l_o' => (int)$l_o,
          'l_b_t' => (int)$l_b_t
          );
      }

      $this->kSA[$key] = (double)$sc;
      if ($this) {
        var_dump((double)$sc, $this->kSA[$key]);
      }
    }
  }
}

function main() {
  $kc = new KM();
  $kc->getInfo();
  var_dump($kc->getKS());
}
main();
