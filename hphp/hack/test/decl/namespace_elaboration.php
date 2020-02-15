<?hh

// The function and const imports don't affect decls, but they're included
// because we have to make sure that they don't cause the direct decl parser to
// return an error.


use \MyNamespace\MyType;
use \MyNamespace\MyType as MyType2;
use function \MyNamespace\dont_care;
use function \MyNamespace\dont_care as really_dont_care;
use const \MyNamespace\DONT_CARE;
use const \MyNamespace\DONT_CARE as REALLY_DONT_CARE;

use \MyNamespace\MyOtherType,
  \MyNamespace\MyOtherType as MyOtherType2,
  function \MyNamespace\other_dont_care,
  function \MyNamespace\other_dont_care as other_really_dont_care,
  const \MyNamespace\OTHER_DONT_CARE,
  const \MyNamespace\OTHER_DONT_CARE as OTHER_REALLY_DONT_CARE;

use \MyNamespace\{
  MyBracedType,
  MyBracedType as MyBracedType2,
  function braced_dont_care,
  function braced_dont_care as braced_really_dont_care,
  const BRACED_DONT_CARE,
  const BRACED_DONT_CARE as BRACED_REALLY_DONT_CARE,
};

use \MyNamespace{
  MyConcatenatedType,
  MyConcatenatedType as MyConcatenatedType2,
  function concatenated_dont_care,
  function concatenated_dont_care as concatenated_really_dont_care,
  const CONCATENATED_DONT_CARE,
  const CONCATENATED_DONT_CARE as CONCATENATED_REALLY_DONT_CARE,
};

namespace {
  use MyNamespace\MyScopedNamespace\MyType;

  function my_scoped_id(MyType $x): MyType2 {
    return $x;
  }
}

function id(MyType $x): MyType2 {
  return $x;
}

function other_id(MyOtherType $x): MyOtherType2 {
  return $x;
}

function braced_id(MyBracedType $x): MyBracedType2 {
  return $x;
}

function concatenated_id(MyConcatenatedType $x): MyConcatenatedType2 {
  return $x;
}
