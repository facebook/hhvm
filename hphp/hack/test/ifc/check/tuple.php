<?hh

class C {
  <<__Policied("S")>>
  public string $s = "";
  <<__Policied("I")>>
  public int $i = 0;
  <<__Policied("J")>>
  public int $j = 0;
  <<__Policied("B")>>
  public bool $b = true;
  <<__Policied("PUBLIC")>>
  public (bool,int) $tuple = tuple(false, 0);
}

<<__InferFlows>>
function ok(C $c): void {
  $tuple = tuple($c->s, $c->i, $c->b, $c->j);
  $c->i = $tuple[1];

  $tuple[3] = $c->i;
  $c->i = $tuple[1];

  $pair = Pair { $c->i, $c->j };
  $c->i = $pair[0];
}

<<__InferFlows>>
function leak_through_access_tuple(C $c): void {
  $tuple = tuple($c->s, $c->i, $c->b, $c->j);
  // J leaks to I
  $c->i = $tuple[3];
}

<<__InferFlows>>
function leak_through_access_pair(C $c): void {
  $pair = Pair { $c->i, $c->j };
  // J leaks to I
  $c->i = $pair[1];
}

<<__InferFlows>>
function leak_through_assignment_tuple(C $c): void {
  $tuple = tuple($c->s, $c->i, $c->b, $c->j);
  $tuple[1] = $c->j;
  // J leaks to I
  $c->i = $tuple[1];
}

// Copy-on-write semantics means `$this->tuple` becomes `tuple(false, $this->i)`
// after the first assignment and thus does not depend on `$this->j` from the
// second assignment.
<<__InferFlows>>
function copyOnWrite(C $c): void {
  $tuple = tuple(false, 0);
  $tuple[1] = $c->i;
  $c->tuple = $tuple; // I leaks to PUBLIC
  $tuple[1] = $c->j; // J does NOT leak to PUBLIC
}

<<__InferFlows>>
function leakPreservedOnAssignment(C $c): void {
  $tuple = tuple($c->i,0);
  $tuple[1] = 1;
  $c->j = $tuple[0];
}
