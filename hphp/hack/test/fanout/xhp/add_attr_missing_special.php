//// base-a.php
<?hh

class :element {
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
  return <element data-x={$prop} />;
}

//// changed-a.php
<?hh

class :element {
  attribute string data-x;
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
  return <element data-x={$prop} />;
}
