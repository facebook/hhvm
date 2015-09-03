//// tosearch.php
<?hh //strict

  class Foo {

    public function bar(): void {
      "A stmt";
      throw new Exception();
    }
  }
//// matcherpattern.php
<?hh //strict

  class Foo {
    public function bar(): void {
      "__SKIPANY";
      {
        throw "__ANY";
        "__ANY";
      }
    }
  }
