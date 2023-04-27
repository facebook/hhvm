//// base-a.php
<?hh

class :element {
  attribute arraykey prop;
  public function __construct(
    public darray<string, dynamic> $attributes,
    public varray<dynamic> $children,
    public string $file,
    public int $line,
  ) {}
}
//// base-child.php
<?hh

class :child extends :element {

}

//// base-gchild.php
<?hh

class :gchild extends :child {
  attribute arraykey prop;
}

//// base-b.php
<?hh

function xhp_simple_attribute(int $prop): :element{
  return <child prop={$prop} />;
}

//// changed-a.php
<?hh

class :element {
  attribute float prop;
  public function __construct(
    public darray<string, dynamic> $attributes,
    public varray<dynamic> $children,
    public string $file,
    public int $line,
  ) {}
}
//// changed-child.php
<?hh

class :child extends :element {

}

//// changed-gchild.php
<?hh

class :gchild extends :child {
  attribute arraykey prop;
}

//// changed-b.php
<?hh

function xhp_simple_attribute(int $prop): :element{
  return <child prop={$prop} />;
}
