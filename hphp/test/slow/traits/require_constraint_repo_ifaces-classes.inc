<?hh

class C1 {
  use ChildRequiresT;
  use ChildProvidesT;
}

class C2 extends Super {
  use ChildRequiresT;
}
