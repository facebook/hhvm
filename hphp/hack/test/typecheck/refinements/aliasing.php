<?hh

class C {
  public ?int $i;
}

function aliasing(C $obj): int {
  $obj->i = 42; // ?int is refined to int

  $alias = $obj;
  $alias->i = null;
  // Refinement for $obj->i should be invalidated here.

  return $obj->i;
}

// Seemingly distinct parameters
function globalAliasing(C $obj1, C $obj2): int {
  $obj1->i = 0;
  $obj2->i = null;

  return $obj1->i;
}

// The following function shows how aliasing arises
function globalAliasingCallsite(): void {
  $c = new C();
  globalAliasing($c,$c);
}

function fullAssignmentInvalidation(C $obj): void {
  $obj->i = 0;
  for ($c = 0; $c < 10; $c++) {
    // The following assignment should invalidate and immediately reintroduce
    // the refinement of $obj->i and type check fine
    $obj->i += 1;
  }
}
