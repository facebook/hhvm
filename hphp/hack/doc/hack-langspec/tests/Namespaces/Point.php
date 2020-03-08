<?hh // strict

namespace Graphics\D2 {

class Point {
  private float $x;
  private float $y;

  public function getX(): float	 	{ return $this->x; }
  public function setX(float $x): void	{ $this->x = $x;   }
  public function getY(): float		{ return $this->y; }
  public function setY(float $y): void	{ $this->y = $y;   }

  public function __construct(float $x = 0.0, float $y = 0.0) {
    $this->x = $x;
    $this->y = $y;
  }

  public function move(float $x, float $y): void {
    $this->x = $x;
    $this->y = $y;
  }	

  public function translate(float $x, float $y): void {
    $this->x += $x;
    $this->y += $y;
  }

  public function __toString(): string {
    return '(' . $this->x . ',' . $this->y . ')';
  }	
}

}
