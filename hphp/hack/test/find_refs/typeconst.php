<?hh

abstract class C {
  abstract const type BAR as string;
}

class D extends C {
  const type BAR = string;
}

class E {
  const type FOO = D;
}

function test(D::BAR $_): E::FOO::BAR {
  return "";
}
