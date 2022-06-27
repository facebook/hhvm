<?hh

function check1 (int $x, int $y, mixed $z1): int {
     ($x === 1) && true; // pointless
     return 1;
}
function check2 (int $x, int $y, mixed $z1): int {
     ($x === 2) || false; // pointless
     return 1;
}
function check3 (int $x, int $y, mixed $z1): int {
     ($x === 3) && false; // always false
     return 1;
}
function check4 (int $x, int $y, mixed $z1): int {
     ($x > 3) && false; // always false
     return 1;
}
function check5 (int $x, int $y, mixed $z1): int {
     true || false; // always true
     return 1;
}
function check6 (int $x, int $y, mixed $z1): int {
     false && ($x === 3); // always false
     return 1;
}
function check7 (int $x, int $y, mixed $z1): int {
     ($x === 3) && ($y === 3); // shouldn't give lint
     return 1;
}
function check8 (int $x, int $y, mixed $z1): int {
     $x > 3 && false; // always false
     return 1;
}
function check9 (int $x, int $y, mixed $z1): int {
     $z1 && false; //pointless boolean should not be detected
     return 1;
}
