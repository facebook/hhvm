<?hh

const dict<string, mixed> DICT = dict['a' => 42];

function main(): void {
  DICT['b1']; // out of bounds T136763758
  DICT['b2'];
}
