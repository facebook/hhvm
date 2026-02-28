<?hh

class A {}

class B extends A {}

function provide_Bs((function(B...): void) $fn): void {
  $fn(new B(), new B());
}

function Bs_to_string(B ...$b): string {
  return "some string";
}

function test(): void {
  /* The following are OK */
  provide_Bs(
    (...$b) ==> {
      Bs_to_string(...$b);
    },
  );

  $lambda = (...$b) ==> {
    Bs_to_string(...$b);
  };

  provide_Bs($lambda);

  /* Where the type is declared we prefer to use that instead, so the following
   * should cause an error.
   */

  provide_Bs(
    (A ...$b) ==> {
      Bs_to_string(...$b);
    },
  );

  $lambda = (A ...$b) ==> {
    Bs_to_string(...$b);
  };
}
