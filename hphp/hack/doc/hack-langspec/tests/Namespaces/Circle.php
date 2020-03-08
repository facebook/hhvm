<?hh // strict

namespace Graphics\D2
{

require_once('Point.php');

class Circle {
  private Point $center;
  private float $radius;

  public function __construct(float $x = 0.0, float $y = 0.0, float $radius = 0.0) {
    $this->center = new Point($x, $y);
    $this->radius = $radius;
  }

  public function __toString(): string {
    return '[' . $this->center . ':' . $this->radius . ']';
  }	
}

}
