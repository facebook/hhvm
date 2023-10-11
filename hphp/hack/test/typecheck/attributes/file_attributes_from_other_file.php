//// file1.php
<?hh
namespace A {
  class X implements \HH\FileAttribute { }
}

//// file2.php
<?hh
<<file: A\X>>

namespace B {
  class BB { }
}
