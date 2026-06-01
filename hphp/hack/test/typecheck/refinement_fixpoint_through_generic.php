<?hh
// Test that recursive class refinement (the fixpoint pattern) works when
// accessed through an intermediate abstract type constant.
//
// Previously, eliminate_recursive_access only handled Tdependent(DTexpr ...)
// as this_ty, but when the receiver is a generic whose name contains "::"
// (e.g. from localizing this::TMyBuilder), ExprDepTy.make_with_dep_kind
// returns it without Tdependent wrapping. This caused a cycle in typing_access
// and produced Tany on the second chained call.

interface IBuilder {
  abstract const type TChainableBuilder as IBuilder with {
    type TChainableBuilder = this::TChainableBuilder; };
  public function next(): this::TChainableBuilder;
}

abstract class OhNo {
  abstract const type TMyBuilder as IBuilder;
  public function noLongerWorks(this::TMyBuilder $builder): void {
    // First call should produce a fixpoint type
    $builder = $builder->next();
    // Second call was previously producing Tany due to a cycle;
    // with the fix, it should produce the same fixpoint type.
    $builder = $builder->next();
  }
}
