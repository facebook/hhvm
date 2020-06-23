<?hh // strict

class C {
  public ?int $i;
  public ?int $j;
}

class D {
  public int $i = 0;
  public int $j = 0;
}

function property_assignment_invalidation(C $objC): void {
  $objD = new D();
  if ($objC->i is nonnull && $objC->j is nonnull) {
    $objD->i = $objC->i;
    // Refinement for $objC->j should be valid here.
    $objD->j = $objC->j;
  }
}
