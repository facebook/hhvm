//// base-impl.php
<?hh

final class Impl implements IChild { use TImpl; }

//// base-child.php
<?hh

interface IChild extends IParent {}

//// base-parent.php
<?hh

interface IParent {
  public function genInt(): Awaitable<int>;
}

//// base-trait.php
<?hh

trait TImpl {
  public async function genInt(): Awaitable<int> {
    return 42;
  }
}
//// changed-impl.php
<?hh

final class Impl implements IChild { use TImpl; }

//// changed-child.php
<?hh

interface IChild extends IParent {}

//// changed-parent.php
<?hh

interface IParent {
  public function genInt(): Awaitable<int>;
}

//// changed-trait.php
<?hh

trait TImpl {
  public async function genInt(): Awaitable<?int> {
    return 42;
  }
}
