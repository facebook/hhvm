//// A.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

module A {}

class :my-xhp-base {
  public function __construct(
    public darray<string,mixed> $attr,
    public varray<mixed> $children,
    public string $filename,
    public int $line_number,
  ) {}
}

<<__Internal>>
class :my-xhp extends :my-xhp-base {}

class :other-xhp extends :my-xhp-base {}

//// main.php
<?hh

function main(): void {
  <my-xhp />; // Not OK
  <other-xhp />; // OK!
}
