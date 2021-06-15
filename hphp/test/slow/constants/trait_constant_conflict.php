<?hh

class C {
  const int X = 3;
}

trait T {
  const string X = "hello";
}

class D extends C {
  use T;
}
