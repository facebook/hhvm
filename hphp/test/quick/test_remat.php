<?hh

class KM {
  private $kSA;
  private $specs;

  function getKS() :mixed{
    return $this->kSA;
  }
  function __construct() {
    $this->specs = dict[
      "r:3600" =>
        dict['oblt' => 0,
          'ms' => 10000000,
          'ttdm' => 100]
      ];
  }

  private static function decay($a, $b, $c, $d) :mixed{ return 0; }

  private function getXXX() :mixed{
    return vec[1, 0, 0];
  }
  public function getInfo() :mixed{
    $time = time();
    $this->kSA = dict[];
    foreach ($this->specs as $key => $spec) {
      $fetched = $this->getXXX();
      list($sc, $l_o, $l_b_t) =
        vec[
          $fetched[0],
          $fetched[1],
          $fetched[2]
        ];

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
        $this->dKV = dict[];
        $this->dKV[$key] = dict[
          'sc' => (float)$sc,
          'l_o' => (int)$l_o,
          'l_b_t' => (int)$l_b_t
          ];
      }

      $this->kSA[$key] = (float)$sc;
      if ($this) {
        var_dump((float)$sc, $this->kSA[$key]);
      }
    }
  }
}

<<__EntryPoint>> function main(): void {
  $kc = new KM();
  $kc->getInfo();
  var_dump($kc->getKS());
}
