//// base-a.php
<?hh
trait A { require extends C; }
//// base-b.php
<?hh
class C {}
class D extends C { use A; }

//// changed-a.php
<?hh
trait A { require extends C; }
//// changed-b.php
<?hh
class C {
  public function foo(): void {}
}
class D extends C { use A; }
