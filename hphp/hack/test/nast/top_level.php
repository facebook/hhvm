<?hh
namespace {
use namespace HH\Lib\{C, Math, Vec};

func_foo();

use function My\Full\func_foo;

foo();

1+foo();

$x = Vec\range(1, 99);

func_foo();

function func_foo() {}
function foo() {}
}

namespace My\Full {
  function func_foo() {}
}

namespace HH\Lib\Vec {
  function range($_, $_) {}
}
