//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    echo "hello something world";
    $hacker = "way";
  }
}

//// matcherpattern.php
<?hh //strict

class __REGEXP { /*F**/
  public function __REGEXP(): void { /*[bar]**/
    echo "__REGEXP"; /*.*something.**/
    $__REGEXP = "way"; /*\$h[a-c]+ker*/
  }
}
