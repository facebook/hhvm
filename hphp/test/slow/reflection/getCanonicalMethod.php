<?hh

interface IForMeth {
  public function meth(): void;
}

trait TForMeth {
  public function meth(): void {}
}

class CForMeth {
  use TForMeth;
}

<<__EntryPoint>>
function get_canonical_method_entrypoint(): mixed {
  $x = new ReflectionMethod('CForMeth::meth');
  dump($x);
  $x = $x->getCanonicalMethod();
  dump($x);
}

function dump(ReflectionMethod $m): void {
  var_dump(
    $m->name,
    $m->class,
    $m->getOriginalClassname(),
    $m->getDeclaringClass()->getName(),
  );
}
