<?hh

class Foo<X> {
  protected ?int $x;
  protected Foo<X> $y;
  protected ?Foo $z;
}

echo "ok";
