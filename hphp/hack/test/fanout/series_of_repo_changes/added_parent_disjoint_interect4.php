//// a.php
<?hh

class A {}

//// b.php
<?hh

class B {}

//// c.php
<?hh

class C extends A {}

//// ta.php
<?hh

trait Ta {
  require extends A;
}

//// tb.php
<?hh

trait Tb {
  use Ta;

  public function foo(): string {
    if ($this is B) {
      return $this;
    }
    return "";
  }
}

////////////////////////

//// a.php
<?hh

class A extends B {}
