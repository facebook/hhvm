<?hh

class C {
  const dict<string, mixed> DICT = dict['a' => 42];
}

class D extends C {
}

function main(): void {
  idx(D::DICT, 'b');
}
