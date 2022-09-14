<?hh

class C {
  const dict<string, mixed> DICT = dict['a' => 42];
}

class D extends C {
}

class E extends C {
  const dict<string, mixed> DICT = dict['b' => true];
}

function main(): void {
  idx(C::DICT, 'c');
  idx(D::DICT, 'd');
  idx(E::DICT, 'e');
}
