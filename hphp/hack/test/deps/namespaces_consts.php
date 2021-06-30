//// file1.php
<?hh

const C = 1;

//// file2.php
<?hh

namespace NS {
  const C = 'a';

  function bar()[]: int {
    return C;
  }
}
