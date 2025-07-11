<?hh

interface IForCnst {
  abstract const type T;
  abstract const type T2 = int;
  const type T3 as arraykey = int;
  const type T4 as arraykey = int;
  const type T5 as arraykey = int;
}

interface I2ForCnst {
  const type T6 as arraykey = int;
}

trait TForCnst implements I2ForCnst {
  const type T = int;
  const type T4 = string; // we still get IForCnst because the trait impl loses
  const type T6 = string;
}

class CForCnst implements IForCnst {
  use TForCnst;
  const type T5 = string;
}

<<__EntryPoint>>
function get_canonical_classname_entrypoint(): mixed {
  foreach (vec['T', 'T2', 'T3', 'T4', 'T5', 'T6'] as $name) {
    $x = new ReflectionTypeConstant('CForCnst', $name);
    var_dump(
      $x->getName(),
      $x->getClass()->getName(),
      $x->getDeclaringClass()->getName(),
      $x->getCanonicalClassname(),
    );
  }
}
