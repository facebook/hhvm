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

//// base-child.php
<?hh

class :child extends :element {}

//// base-b.php
<?hh

function xhp_simple_attribute(int $prop): :child {
  return <child prop={$prop} />;
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

//// changed-child.php
<?hh

class :child extends :element {}

//// changed-b.php
<?hh

function xhp_simple_attribute(int $prop): :child {
  return <child prop={$prop} />;
}
