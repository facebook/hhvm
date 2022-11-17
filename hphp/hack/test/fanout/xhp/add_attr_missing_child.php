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

//// base-child.php
<?hh

class :child extends :element {

}

//// base-b.php
<?hh

function xhp_simple_attribute(int $prop): :element{
  return <child prop={$prop} />;
}

//// changed-a.php
<?hh

class :element {
  attribute string prop;
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

//// changed-b.php
<?hh

function xhp_simple_attribute(int $prop): :element{
  return <child prop={$prop} />;
}
