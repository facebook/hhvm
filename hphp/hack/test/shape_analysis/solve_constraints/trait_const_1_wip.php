<?hh

trait TheTrait {
  // T136213776: expect ?b key in shape type
  const dict<string, mixed> DICT = dict[];
}

class TheClass {
  use TheTrait;
}

function main(): void {
  TheClass::DICT['b'];
}
