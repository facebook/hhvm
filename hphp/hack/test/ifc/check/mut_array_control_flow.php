<?hh

class C {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("I")>>
    public int $i,
    <<__Policied("J")>>
    public int $j,
  ) {}
}

<<__InferFlows>>
function ok_vector_ix(C $c, Vector<int> $vector): void {
  $vector[] = $c->i; // Force lump of vector to be I

  $c->j = 0; // OK because vector append does not raise an exception
}

<<__InferFlows>>
function ok_map_extend(C $c, Map<string,int> $map): void {
  $map['hello'] = $c->i; // Force lump of vector to be I

  $c->j = 0; // OK because map extension does not raise an exception
}

<<__InferFlows>>
function leak_via_exn(C $c, Vector<int> $vector): void {
  $vector[0] = $c->i; // Force lump of vector to be I

  // There is a flow from PC which is governed by length of vector due to the
  // conditional exception. The length is governed by the lump policy.
  $c->j = 0; // I flows into J
}
