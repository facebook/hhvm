<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

async function nullable_bool(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(): ?ExampleBool)>> {
  throw new Exception();
}

async function a_bool(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(): ExampleBool)>> {
  throw new Exception();
}

/**
 * Since all Hack types are truthy, typically, most syntactic places that
 * expect booleans allow all types. However, as to not leak these truthy
 * Hack semantics to Expression Trees, ensure that those syntactic positions
 * only accept types that may be coerced to booleans.
 */
function test(): void {
  $y = ExampleDsl`
    () ==> {
      // Boolean ||
      nullable_bool() || a_bool();
      a_bool() || nullable_bool();
      a_bool() || a_bool();

      // Boolean &&
      nullable_bool() && a_bool();
      a_bool() && nullable_bool();
      a_bool() && a_bool();

      // Boolean !
      !nullable_bool();
      !a_bool();
    }
  `;
}
