<?hh

class A {
  <<__Policied("PUBLIC")>>
  public ?int $pub = 0;
  <<__Policied("PRIVATE")>>
  public ?int $pri = 0;
}

<<__InferFlows>>
function wpub(A $a): int {
  $a->pub = 10;
  return 10;
}

<<__InferFlows>>
function wpri(A $a): int {
  $a->pri = 10;
  return 10;
}

<<__InferFlows>>
function ok(A $a): void {
  $a->pri ?? wpri($a);
  $a->pri ?? wpri($a);
  $a->pri ?: wpri($a);
  ($a->pub ?? wpub($a)) + wpri($a);
  $a->pub ?? wpri($a);
  $a->pub ? wpub($a) : wpri($a);
  $a->pub ? wpri($a) : wpub($a);
}

<<__InferFlows>>
function ko0(A $a): void {
  $a->pri ?? wpub($a);
}

<<__InferFlows>>
function ko1(A $a): void {
  $a->pri ? 0 : wpub($a);
}

<<__InferFlows>>
function ko2(A $a): void {
  $a->pub = ($a->pri ?? 0);
}

<<__InferFlows>>
function ko3(A $a, int $i): void {
  $a->pub = ($i ?? $a->pri);
}

<<__InferFlows>>
function ko4(A $a, bool $b): void {
  $a->pub = ($b ? $a->pri : 0);
}

<<__InferFlows>>
function ko5(A $a, bool $b): void {
  $a->pub = ($b ? 0 : $a->pri);
}
