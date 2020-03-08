<?hh // strict

namespace Graphics;

class Point implements \Serializable {	// note the interface
  private static int $nextId = 1;
  private int $id;	// transient property; not serialized

  private float $x;
  private float $y;

  public function __construct(float $x = 0.0, float $y = 0.0) {
    $this->x = $x;
    $this->y = $y;
    $this->id = self::$nextId++;
  }

  public function __toString(): string {
    return 'ID:' . $this->id . '(' . $this->x . ',' . $this->y . ')';
  }

  public function serialize(): string {
    return serialize(array('y' => $this->y, 'x' => $this->x));
  }

  public function unserialize(string $sdata): void {
    $data = unserialize($sdata);
    $this->x = $data['x'];
    $this->y = $data['y'];
    $this->id = self::$nextId++;
  }
}
