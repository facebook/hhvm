<?hh
//var_dump(token_get_all("<?hh const super = 0;"));

class TypeConstraintWhereClausesTest<T> {
  // where should not be a keyword where it could be an identifier
  const where = 0;
  private T $where;
  public function where(): void {}

  public function typeConstraintOnly(): void where T = int {}

  public function typeVariableInResult<Tx>(): Tx where T = ?Tx {
    if (null === $this->where) {
      throw new Exception('Nowhere to run');
    }
    return $this->where;
  }

  public function typeVariableInArgsAndResult(Tx $alt): Tx where T = ?Tx {
    return $this->where ?: $alt;
  }

  public function complexTypesInConstraints(
  ): void where Map<int, string> super Vector<bool> {}

  public function multipleConstraintsInWhereClause<Tinner, Tkey, Tvalue>(
  ): void where
    T = ?Tinner,
    Tinner as ConstMap<Tkey, Tvalue>,
    Tkey as IComparable<Tkey>,
  {}
}

// Where clauses should be supported on toplevel functions
function whereClauseOnFunction<T> (): T where T=int {
    return 5;
}
