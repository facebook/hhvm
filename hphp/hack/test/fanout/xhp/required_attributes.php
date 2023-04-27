//// base-a.php
<?hh

class :element {
  attribute int prop @required;
  public function __construct(
    public darray<string, dynamic> $attributes,
    public varray<dynamic> $children,
    public string $file,
    public int $line,
  ) {}
}
//// base-b.php
<?hh

function xhp_simple_attribute(int $prop): :element{
  return <element prop={$prop} />;
}

//// changed-a.php
<?hh

class :element {
  attribute int prop @required, string prop2 @required;
  public function __construct(
    public darray<string, dynamic> $attributes,
    public varray<dynamic> $children,
    public string $file,
    public int $line,
  ) {}
}
//// changed-b.php
<?hh

function xhp_simple_attribute(int $prop): :element{
  return <element prop={$prop} />;
}
