<?hh

class C {
  const type BAR = string;
}

class D {
  const type FOO = C;
}

function test(): D::FOO::BAR {
  return "";
}
