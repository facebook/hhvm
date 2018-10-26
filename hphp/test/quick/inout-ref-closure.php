<?hh
$closure = (&$a, &$b, &$c) ==> {
  print("In closure: $a, $b, $c\n");
};

$a = 'hello';
$b = 'world';
$c = 8;

$closure(
  inout $a,
  inout $b,
  inout $c,
);
print("After closure: $a, $b, $c\n");
