<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

async function nullable_bool(
  ExampleContext $_,
): Awaitable<ExprTree<Code, Code::TAst, (function(): ?ExampleBool)>> {
  throw new Exception();
}

async function a_bool(
  ExampleContext $_,
): Awaitable<ExprTree<Code, Code::TAst, (function(): ExampleBool)>> {
  throw new Exception();
}

/**
 * Since all Hack types are truthy, typically, most syntactic places that
 * expect booleans allow all types. However, as to not leak these truthy
 * Hack semantics to Expression Trees, ensure that those syntactic positions
 * only accept booleans, rather than any truthy expression.
 */
function test(): void {
  $y = Code`
    () ==> {
      // if/else
      if (nullable_bool()) {}
      if (nullable_bool()) {} else {}

      if (a_bool()) {}
      if (a_bool()) {} else {}

      if (a_bool()) {}
      else if (nullable_bool()) {}

      if (a_bool()) {}
      else if (nullable_bool()) {}
      else {}

      if (a_bool()) {}
      else if (a_bool()) {}
      else {}

      // while() {}
      while(nullable_bool()) {}
      while(a_bool()) {}

      for (;nullable_bool();) {}
      for ($i = 0; nullable_bool();) {}
      for (; nullable_bool(); $i = $i + 1) {}
      for ($i = 0; nullable_bool(); $i = $i + 1) {}

      for (;a_bool();) {}
      for ($i = 0; a_bool();) {}
      for (; a_bool(); $i = $i + 1) {}
      for ($i = 0; a_bool(); $i = $i + 1) {}

      // ternary _ ? _ : _ operator
      nullable_bool() ? 1 : 2;
      a_bool() ? 1 : 2;

      "not a bool" ? 1 : 2;

      // Represents an infinite loop and should be the last item
      // As the typechecker may not throw type errors afterwards,
      // due to the flow sensitive nature of the typechecker.
      for (;;) {}
    }
  `;
}
