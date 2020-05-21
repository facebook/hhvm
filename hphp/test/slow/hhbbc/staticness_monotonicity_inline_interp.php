<?hh

// iteration 1: SVecE~vec()
function emptyvec(): vec<string> {
  return vec[];
}

// iteration 1: Vec
// iteration 2: SVecE~vec()
function emptyvec2(): vec<string> {
  return emptyvec();
}

// iteration 1: Vec(SStr="a")
// iteration 3 inlined from bar: VecN(SStr="a")
function foo(vec<string> $vec, bool $a): vec<string> {
  $vec = vec[];
  if ($a) $vec[] = 'a';
  return $vec;
}

// iteration 1: Str
// iteration 2: SStr (because foo returns Vec(SStr="a"))
// iteration 3: SStr (because foo correctly returns VecN(SStr="a"))
function bar(): string {
  $store = abs(rand()) >= 0 ? foo(emptyvec2(), !emptyvec2()) : vec[];
  return $store[0];
}

<<__EntryPoint>>
function main(): void {
  var_dump(bar());
}
