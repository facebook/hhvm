<?hh

const dict<string, mixed> DICT1 = dict['a' => 42];
const dict<string, mixed> DICT2 = dict['a' => true];

function main(): void {
  DICT1['b'];
  DICT2['b'];
}
