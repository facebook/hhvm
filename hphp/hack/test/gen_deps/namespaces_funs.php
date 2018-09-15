//// file1.php
<?hh
function foo(): string { return ''; }

namespace NS {
  function foo(): int { return 0; }
}

//// file2.php
<?hh
namespace NS {
  function bar(): int {
    return foo();
  }
}
