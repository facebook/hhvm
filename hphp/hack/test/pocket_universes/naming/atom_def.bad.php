//// file1.php
<?hh // strict

class C extends B {
  enum F {
  :@C(
    /* type T = int, */ /* missing type */
    type S = string, /* unknown type */
    name = "C",
    data = 4,
    bar = 5.0
  );
  }
}

//// file2.php
<?hh // strict

class B extends A {
  enum F {
    case float bar;
    /* missing extension of PU definition from parent */
    /* :@A( */
    /*   bar = 3.14 */
    /* ); */
    :@B(
      type T = string,
      /* name = "A", */ /* missing value */
      data = "42",
      yolo = "foo", /* unknown value */
      bar = 16.64
    );
  }
}

//// file3.php
<?hh // strict

class A {
  enum F {
    case type T;
    case string name;
    case T data;

    :@A(
      type T = int,
      name = "A",
      data = 42
    );
  }
}
