<?hh
<<file: __EnableUnstableFeatures('case_types')>>

// Recursive, so HHBBC can't resolve type constants that reach it until a later
// round.
case type ClientMixed = int | vec<ClientMixed>;

type TLoadResult = shape('items' => vec<ClientMixed>);

final class Params<T> {}

class Builder<TController, TParams> {
  public static function print(): void {
    echo "builder\n";
  }
}

abstract class PaginationController {
  // Makes HHBBC analyze this class again, but not RelatedPostsController, once
  // ClientMixed is resolved.
  const int UNUSED = UndefinedClass::VALUE;

  const type TReturn = TLoadResult;
  abstract const type TBKS2Params as Params<this::TReturn> = Params<this::TReturn>;
  abstract const type TParams = mixed;
  abstract const type TBuilder as Builder<this, this::TBKS2Params> = Builder<this, this::TBKS2Params>;

  // Not inlined, so the JIT only knows static <= PaginationController.
  <<__NEVER_INLINE>>
  public static function getBuilderClassnameOverride(): class<this::TBuilder> {
    return HH\ReifiedGenerics\get_class_from_type<this::TBuilder>();
  }
}

final class RelatedPostsController extends PaginationController {
  const type TParams = int;
}

<<__EntryPoint>>
function main(): void {
  $builder = RelatedPostsController::getBuilderClassnameOverride();
  $builder::print();
}
