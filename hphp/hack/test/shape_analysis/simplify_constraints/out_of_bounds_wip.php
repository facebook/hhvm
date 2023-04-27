<?hh

function main(): void {
  $d = dict['a' => 1];
  $d['b']; // out of bounds T136763758
}
