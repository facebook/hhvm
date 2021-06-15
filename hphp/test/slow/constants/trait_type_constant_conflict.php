<?hh

class C {
  const type Ty = int;
}

trait T {
  const type Ty = string;
}

class D extends C {
  use T;
}
