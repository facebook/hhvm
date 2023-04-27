<?hh

class C {
  const dict<string, mixed> DICT = dict['a' => 42];
}

class D extends C {
  const dict<string, mixed> DICT = dict['b' => 42];
}

class E extends D {
  const dict<string, mixed> DICT = dict['c' => 42];
}

function main(): void {
  idx(E::DICT, 'd');
  idx(D::DICT, 'e');
}
