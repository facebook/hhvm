//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    echo "hello something world";
    $a = "052";
  }
}

//// matcherpattern.php
<?hh //strict

class Foo { /*F+*/
  public function bar(): void {
    echo "__REGEXP"; /*.*something.**/
    $__ANY = "__REGEXP"; /*[0-9]+*/
  }
}
