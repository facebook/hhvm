//// base-impl.php
<?hh

class Impl extends CParent {
  use MyTrait;
}

//// base-trait.php
<?hh

trait MyTrait {
  public function genInt(): int {
    return 42;
  }
}

//// base-parent.php
<?hh

class CParent {
  public function genInt(): int {
    return 42;
  }
}
//// changed-impl.php
<?hh

class Impl extends CParent {
  use MyTrait;
}

//// changed-trait.php
<?hh

trait MyTrait {
  public function genInt(): ?int {
    return 42;
  }
}

//// changed-parent.php
<?hh

class CParent {
  public function genInt(): int {
    return 42;
  }
}
