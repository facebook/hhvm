//// base-parent.php
<?hh
abstract class :base {
  public function __construct(
    public darray<string, dynamic> $attributes,
    public varray<dynamic> $children,
    public string $file,
    public int $line,
  ) {}
}

//// base-a.php
<?hh

abstract class :element extends :base {
  attribute int prop @required;
}

//// base-child.php
<?hh

final class :child extends :element {}

//// base-b.php
<?hh

function xhp_simple_attribute(int $prop): :child{
  return <child prop={$prop} />;
}

//// changed-parent.php
<?hh
abstract class :base {
  public function __construct(
    public darray<string, dynamic> $attributes,
    public varray<dynamic> $children,
    public string $file,
    public int $line,
  ) {}
}

//// changed-a.php
<?hh

abstract class :element extends :base {
  attribute int prop @required, string prop2 @required;
}

//// changed-child.php
<?hh

final class :child extends :element {}

//// changed-b.php
<?hh

function xhp_simple_attribute(int $prop): :child{
  return <child prop={$prop} />;
}
