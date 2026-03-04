<?hh

interface Box {
  abstract const type T;
}

// Type alias with exact refinement
type ExactBox = Box with { type T = int };

// Type alias with upper bounded refinement
type BoundedBox = Box with { type T as arraykey };

// Type alias with lower bounded refinement
type LowerBoundedBox = Box with { type T super int };

// Interface with abstract context constant
abstract class BoxWithCtx {
  abstract const ctx C;
}

// Type alias with exact ctx refinement
type PureBox = BoxWithCtx with { ctx C = [defaults] };

// Type alias with upper bounded ctx refinement
type BoundedCtxBox = BoxWithCtx with { ctx C as [defaults] };

// Class with type constant using refinement
class C {
  const type TExact = Box with { type T = string };
}

<<__EntryPoint>>
function main(): void {
  echo "=== Type alias with exact refinement ===\n";
  $ts = type_structure('ExactBox');
  var_dump($ts['kind']); // should be interface kind (16)
  var_dump($ts['classname']); // should be Box
  var_dump($ts['with_refinements']['T']['equals']['kind']); // should be int (1)
  var_dump($ts['with_refinements']['T']['is_ctx']); // should be false

  echo "\n=== Type alias with bounded refinement ===\n";
  $ts = type_structure('BoundedBox');
  var_dump($ts['kind']); // should be interface kind (16)
  var_dump(isset($ts['with_refinements']['T']['as'])); // should be true
  var_dump($ts['with_refinements']['T']['as'][0]['kind']); // should be arraykey (7)

  echo "\n=== Type alias with lower bounded refinement ===\n";
  $ts = type_structure('LowerBoundedBox');
  var_dump($ts['kind']); // should be interface kind (16)
  var_dump(isset($ts['with_refinements']['T']['super'])); // should be true
  var_dump($ts['with_refinements']['T']['super'][0]['kind']); // should be int (1)

  echo "\n=== Type alias with exact ctx refinement ===\n";
  $ts = type_structure('PureBox');
  var_dump($ts['kind']); // should be class kind (15)
  var_dump($ts['with_refinements']['C']['is_ctx']); // should be true
  var_dump(isset($ts['with_refinements']['C']['equals'])); // should be true

  echo "\n=== Type alias with bounded ctx refinement ===\n";
  $ts = type_structure('BoundedCtxBox');
  var_dump($ts['kind']); // should be class kind (15)
  var_dump($ts['with_refinements']['C']['is_ctx']); // should be true
  var_dump(isset($ts['with_refinements']['C']['as'])); // should be true

  echo "\n=== Type constant with exact refinement ===\n";
  $ts = type_structure(C::class, 'TExact');
  var_dump($ts['kind']); // should be interface kind (16)
  var_dump($ts['with_refinements']['T']['equals']['kind']); // should be string (4)

}
