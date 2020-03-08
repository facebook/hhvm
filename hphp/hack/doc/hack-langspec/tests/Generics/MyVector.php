<?hh // strict

namespace NS_MyVector;

class MyVector<T> {
  private int $length;
  private array<T> $vector;

  public function getLength(): int {
    return $this->length;
  }

  private function setLength(int $newLength): void {
    $this->length = $newLength;
  }

  public function getElement(int $index): T {
    // bounds checking omitted
    return $this->vector[$index];
  }

  public function setElement(int $index, T $newValue): void {
    // bounds checking omitted

    $this->vector[$index] = $newValue;
  }

  public function __construct(int $vectorLength, T $initValue) {
    $this->length = $vectorLength;
    $this->vector = array();
    for ($i = 0; $i < $this->length; ++$i) {
      $this->vector[] = $initValue;
    }
  }

  public function __toString(): string {
    $s = '[';
    for ($i = 0; $i < $this->length - 1; ++$i) {
      $s .= (string)$this->vector[$i] . ':';
    }
    $s .= (string)$this->vector[$i] . ']';
    return $s;
  }
}
