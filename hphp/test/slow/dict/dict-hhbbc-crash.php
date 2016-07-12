<?hh

function main() {
  $d1 = dict[]; $d1[false] = 1;
  $d2 = dict[0=>0]; $d2[false][0][0] = 1;
}

main();
