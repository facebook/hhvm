<?hh

const dict<string, mixed> DICT1 = dict['a' => 42];
const dict<string, mixed> DICT2 = dict['a' => true];

function main(): void {
  DICT1['b']; // out of bounds T136763758
  DICT2['b'];
}
