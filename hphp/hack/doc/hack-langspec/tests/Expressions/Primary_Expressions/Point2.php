<?hh // strict

namespace NS_intrinsics_exit;

class Point2 {
  private static int $pointCount = 0;

  public float $x;		// Cartesian x-coordinate
  public float $y;		// Cartesian y-coordinate

  public static function getPointCount(): int {
    return self::$pointCount;
  }

  public function __construct(float $x = 0.0, float $y = 0.0) {
    $this->x = $x;
    $this->y = $y;
    ++self::$pointCount;
  }

  public function __destruct() {
    --self::$pointCount;
  }

  public function __toString(): string {
    return '(' . $this->x . ',' . $this->y . ')';
  }	
}
