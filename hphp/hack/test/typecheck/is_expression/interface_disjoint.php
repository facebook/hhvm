<?hh

class C {}
interface IC1 {
  require extends C;
}
interface IC2 {
  require extends C;
}

<<__Sealed(T::class)>>
interface SealedI {
}

trait T {
  require extends C;
}

interface INotC {
  require extends NotC;
}
class NotC {}

function test1(IC1 $i1): nothing {
  if (!($i1 is IC2)) {
    // Error: Should not be nothing
    return $i1;
  }
  while (true) {
  }
}

function test2(IC1 $i1): nothing {
  if (!($i1 is SealedI)) {
    // Error: Should not be nothing
    return $i1;
  }
  while (true) {
  }
}

function test3(INotC $nc): void {
  if (!($nc is IC1)) {
    // Ok: Since IC1 & INotC are disjoint, a negation type isn't required
    hh_expect_equivalent<INotC>($nc);
  } else {
    // Will be ok after legacy refinement replace
    // hh_expect<nothing>($nc);
  }
  if (!($nc is IC2)) {
    // Ok: Since IC2 & INotC are disjoint, a negation type isn't required
    hh_expect_equivalent<INotC>($nc);
  } else {
    // Will be ok after legacy refinement replace
    // hh_expect<nothing>($nc);
  }
  if (!($nc is SealedI)) {
    // Ok: Since SealedI & INotC are disjoint, a negation type isn't required
    hh_expect_equivalent<INotC>($nc);
  } else {
    // Will be ok after legacy refinement replace
    // hh_expect<nothing>($nc);
  }
}
