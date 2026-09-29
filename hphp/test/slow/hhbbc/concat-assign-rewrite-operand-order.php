<?hh

// HHBBC rewrites `$l = $l . $x` into `$l .= $x`. Concat converts its left
// operand to string first and ConcatEqual its right, so when converting the
// local can throw the rewrite changes which error the program sees: here a
// catchable InvalidOperationException became an object-to-string fatal.
//
// Each value arrives as a typed parameter, laundered at the call, so that
// HHBBC knows the local's type but not its contents.  Both halves matter: a
// known value is folded away before the rewrite is reached, and an unknown
// type makes the rewrite decline for a different reason than the one under
// test.  Taking it as a parameter is also what keeps the local alive -- assign
// a call's result to a fresh local and HHBBC drops the local entirely, leaving
// no `$l = $l . $x` to rewrite.

class ConcatC {}
function concat_fn(): void {}

function concat_dbl(float $l): string { $l = $l . new ConcatC(); return $l; }
function concat_vec(vec<int> $l): string { $l = $l . new ConcatC(); return $l; }
function concat_dict(dict<string, int> $l): string {
  $l = $l . new ConcatC();
  return $l;
}
function concat_func((function(): void) $l): string {
  $l = $l . new ConcatC();
  return $l;
}
function concat_str(string $l): string { $l = $l . 'b'; return $l; }

function show(string $name, (function(): string) $f): void {
  try {
    $r = $f();
  } catch (\Throwable $e) {
    $r = \get_class($e);
  }
  echo $name, ": ", $r, "\n";
}

<<__EntryPoint>>
function main_concat_assign_rewrite_operand_order(): void {
  show('dbl', () ==> concat_dbl(__hhvm_intrinsics\launder_value(-\INF)));
  show('vec', () ==> concat_vec(__hhvm_intrinsics\launder_value(vec[1, 2])));
  show('dict',
       () ==> concat_dict(__hhvm_intrinsics\launder_value(dict['a' => 1])));
  show('func',
       () ==> concat_func(__hhvm_intrinsics\launder_value(concat_fn<>)));
  show('str', () ==> concat_str(__hhvm_intrinsics\launder_value('a')));
}
