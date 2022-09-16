<?hh

class C {
  const dict<string, mixed> DICT = dict['a' => 42];
}

function main(): void {
  idx(C::DICT, 'b');
}
