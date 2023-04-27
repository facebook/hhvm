//// base-impl.php
<?hh

class Impl extends CParent implements IParent {}

//// base-iparent.php
<?hh

interface IParent {
  public function genInt(): int;
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

class Impl extends CParent implements IParent {}

//// changed-iparent.php
<?hh

interface IParent {
  public function genInt(): int;
}

//// changed-parent.php
<?hh

class CParent {
  public function genInt(): ?int {
    return 42;
  }
}
