<?hh // strict

function foo(dynamic $d, ?int $i, num $n): void {
  5 === $d;
  5 !== $i;
  5 === $n;
  $d !== 5;
  $i === 5;
  $n !== 5;
  $i === $n;
  $n !== $i;
  5 === 5;
  $d !== $d;
  5 === "string";
  "string" !=== 5;
}
