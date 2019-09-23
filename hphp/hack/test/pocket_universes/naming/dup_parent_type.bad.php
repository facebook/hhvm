//// file1.php
<?hh // strict

class C extends B {
  enum E {
    case type T;
    case type S;
    case int foo;
  }
  enum F {
    case type T;
    case T foo;
  }
}

//// file2.php
<?hh // strict

class B extends A {
  enum F {
    case type T;
    case T foo;
    case string key;
  }
}

//// file3.php
<?hh // strict

class A {
  enum F {
    case type T;
    case type S;
    case string key;

    :@A( type T = int, key = "A" );
  }
}
