<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('expression_tree_nested_bindings')>>

function g(
  ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $a,
  string $b,
): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  return $a;
}

function h(
  int $b,
  ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $a,
): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  return $a;
}

function i(int $b): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  return ExampleDsl`1`;
}

function id<T>(T $x): T {
  return $x;
}

function f(): void {
  // Each of these should have an error because the binding is in an ET and the use in a splice
  ExampleDsl`$x ==> ${i($x)}`;
  ExampleDsl`$x ==> ${h($x, ExampleDsl`$x`)}`;
  ExampleDsl`$x ==> ${g(ExampleDsl`$x`, $y)}`;
  ExampleDsl`$y ==> ${g(ExampleDsl`$x`, $y)}`;
  ExampleDsl`$x ==> ${g(ExampleDsl`1 + $x`, $x + 1)}`;
  ExampleDsl`$x ==> ${g(ExampleDsl`$x`, $x)}`;
  ExampleDsl`$x ==> ${g(ExampleDsl`$x`, id($x))}`;
  ExampleDsl`$x ==> ${g(ExampleDsl`1 + $x`, $x)}`;
  ExampleDsl`$x ==> ${g(ExampleDsl`1 + $x`, $x.'')}`;
  ExampleDsl`{
    $x = 1;
    return 1 +
      ${
        ExampleDsl`{
          $y = 0 + ${$x};
          return $y + $x;
        }`
      };
  }`;

  // These should have an error because the binding is in a splice and the use in an ET
  ExampleDsl`${($x ==> ExampleDsl`$x`)(1)}`;
  ExampleDsl`${
    (
      $x ==> {
        $y = 1;
        return ExampleDsl`$y`;
      }
    )(1)
  }`;

  // Shouldn't have an error, because the $z bindings from the ET and outside shouldn't interfere
  $z = ExampleDsl`1`;
  ExampleDsl`{
    $z = 1;
    return ${
      ExampleDsl`{
        0 + ${$z};
        0 + $z;
        return 1;
      }`
    };
  }`;
}
