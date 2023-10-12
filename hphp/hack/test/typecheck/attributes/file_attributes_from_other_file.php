//// file1.php
<?hh // strict
namespace A {
  class X implements \HH\FileAttribute { }
}

//// file2.php
<?hh // strict
<<file: A\X>>

namespace B {
  class BB { }
}
