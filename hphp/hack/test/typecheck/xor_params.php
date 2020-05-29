<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function nullthrows<T>(?T $x): T {
  return $x as nonnull;
}

final class XorParam<Tx, Ty> {
  private function __construct(private ?Tx $x, private ?Ty $y) {
    invariant(
      ($x === null) !== ($y === null),
      "Exactly one input value must be null!",
    );
  }

  public static function fromFirstValue(?Tx $x, ?Ty $y): XorParam<Tx, Ty> {
    if ($x !== null) {
      return new XorParam($x, null);
    }
    return new XorParam(null, $y);
  }

  public function getX(): ?Tx {
    return $this->x;
  }

  public function atX(): Tx {
    return nullthrows($this->x);
  }

  public function getY(): ?Ty {
    return $this->y;
  }

  public function atY(): Ty {
    return nullthrows($this->y);
  }

  // Returns the single non-null value
  // TODO: is it possible to augment the type info within the generics so we
  // can type this return value?
  public function getValue(): mixed {
    $z = $this->x ?? $this->y;
    $w = nullthrows($z);
    return $w;
  }
}
