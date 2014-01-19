<?

// Our own rand, for determinism and consistency across implementations
// and versions. See Wikipedia's Linear_congruential_generator article.
define('vaxRandA', 1664525);
define('vaxRandC', 1013904223);
function vaxRand($min, $max) {
  static $seed = 10;
  $seed = (($seed * vaxRandA) + vaxRandC) % (1 << 32);
  return $min + ($seed % ($max - $min));
}

class Point {
  public $x;
  public $y;
  public $z;
  public function __construct( ) {
    $this->x = vaxRand(0, 100000);
    $this->y = vaxRand(0, 100000);
    $this->z = vaxRand(0, 100000);
  }
  public function output($i) {
    printf("%4d: (%3d, %3d, %3d)\n", $i,
           $this->x, $this->y, $this->z);
  }
}

function avgMemb($pts, $memb) {
  $total = 0;
  $count = 0;
  foreach ($pts as $pt) {
    $total += $pt->$memb;
    $count++;
  }
  return $total / $count;
}

function center($n) {
  $pts = array();
  for ($i = 0; $i < $n; ++$i) {
    $p =new Point();
    $pts[] = $p;
  }

  $center = new Point();
  $center->x = avgMemb($pts, 'x');
  $center->y = avgMemb($pts, 'y');
  $center->z = avgMemb($pts, 'z');
  return $center;
}

for ($i = 1000; $i < 1000000; $i = $i + 1 + ($i / 2)) {
  $center = center($i);
  $center->output($i);
}
