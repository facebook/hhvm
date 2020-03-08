<?hh // strict

namespace NS_MyRangeException;

class MyRangeException extends \Exception {
  private int $badValue;
  private int $lowerValue;
  private int $upperValue;

  public function __construct(string $message, int $badValue, int $lowerValue, int $upperValue) {
    parent::__construct($message);

    $this->badValue = $badValue;
    $this->lowerValue = $lowerValue;
    $this->upperValue = $upperValue;
  }

  public function getBadValue(): int   { return $this->badValue; }
  public function getLowerValue(): int { return $this->lowerValue; }
  public function getUpperValue(): int { return $this->upperValue; }

  public function __toString(): string {
    return parent::__toString()
      . ", badValue: " . $this->badValue
      . ", lowerValue: " . $this->lowerValue
      . ", upperValue: " . $this->upperValue;
  }
}
