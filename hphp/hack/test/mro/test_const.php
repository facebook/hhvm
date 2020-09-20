<?hh
interface I {
  const x = 5;
}

trait T implements I {
}

class C {
  use T;
}
