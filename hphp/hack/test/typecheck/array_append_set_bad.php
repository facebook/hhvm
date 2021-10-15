<?hh

function f(Set<int> $s) : void {
  $s[] = "1";
}

// Allowing the above is a typehole
function expect_int(int $i) : void { }

<<__EntryPoint>>function main() : void {
  $s = Set<int>{};
  f($s);
  foreach ($s as $i) {
    expect_int($i);
  }
}

function f2() : void {
  $s = Set{};
  $s[] = 1.1;
}
