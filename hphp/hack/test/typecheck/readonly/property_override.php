<?hh

class ParentMutable {
  public int $a = 0;
  public readonly int $b = 0;
}

// Overriding changes readonly-ness in both directions: both warn.
class ChildFlip extends ParentMutable {
  public readonly int $a = 0;
  public int $b = 0;
}

// Overriding preserves readonly-ness: no warning.
class ChildSame extends ParentMutable {
  public int $a = 0;
  public readonly int $b = 0;
}
