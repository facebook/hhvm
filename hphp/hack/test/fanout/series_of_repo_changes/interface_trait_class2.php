//// i.php
<?hh

interface I {
  public function f(): void;
}

//// t1.php
<?hh

trait T1 implements I {
  <<__Override>>
  public function f(): void;
}

//// t2.php
<?hh

trait T2 {
  use T1;

  <<__Override>>
  public function f(): void;
}

//// c.php
<?hh

final class C {
  use T2;
}

///////////////////////////

//// i.php
<?hh

interface I {}
