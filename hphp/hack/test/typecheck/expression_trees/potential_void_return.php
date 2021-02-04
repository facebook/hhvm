<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

async function potential_void_return<T>(
  ExampleContext $_,
): Awaitable<ExprTree<Code, Code::TAst, (function((function(): T)): T)>> {
  return Code`((function(): T) $x): T ==> {
    return $x();
  }`;
}

function test(): void {
  Code`
    (ExampleBool $b) ==> {
      // Lambda that return ExampleVoid
      $x = () ==> {};
      if ($b) {
        return potential_void_return($x);
      }
      // We do not append a virtualized `return;` here because we are unable to
      // determine during virtualization whether this a void return lambda
      // Thus, we get an inconvenient type error, but it has been deemed
      // okay for developers to append `return;` here themselves as a compromise
    }
  `;
}

function test2(): void {
  Code`
    (ExampleBool $b) ==> {
      // Lambda that returns ExampleVoid
      $x = () ==> {};
      if ($b) {
        return potential_void_return($x);
      }
      // Developers currently need to write this return themselves rather
      // than rely on virtualization to write it there for them
      return;
    }
  `;
}
