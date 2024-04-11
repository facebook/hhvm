//// i.php
<?hh

interface I {
  public function f(): void;
}

//// t.php
<?hh

trait T implements I {
  <<__Override>>
  public function f(): void;
}

//// c.php
<?hh

final class C {
  use T;
}

///////////////////////////

//// i.php
<?hh

interface I {}
