<?hh

const dict<string, mixed> DICT = dict['a' => 42];

function main(): void {
  DICT['b']; // out of bounds T136763758
}
