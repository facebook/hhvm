<?hh

class :xhp {
  attribute string target;

  public function __construct(
    public darray<string,mixed> $a, // Attributes
    public varray<mixed> $b, // Children
    public string $c, // Filename
    public int $d, // Line number
  ) {}
}

function main(?string $s): void {
  <xhp target={$s as nonnull} />;
