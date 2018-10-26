<?hh
function func(inout $a, inout $b, inout $c): void {
  print("In func: $a, $b, $c\n");

}

$a = 'hello';
$b = 'world';
$c = 8;
func(
  inout $a,
  inout $b,
  inout $c,
);
print("After func: $a, $b, $c\n");
