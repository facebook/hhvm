<?hh
function okay() {
  $a = "$b";
  $a = "$b->c";
  $a = "$b[0]";
  $a = "$b[$c]";
  $a = "$$b";

  // Note: only `$b->c` is interpolated, and `->d` is left as a literal string.
  $a = "$b->c->d";
  // Note: only `$b[0]` is interpolated, and `[1]` is left as a literal string.
  $a = "$b[0][1]";
  // Note: only `$b` is interpolated, and `\[0]` is left as a literal string.
  $a = "$b\[0]";

  $a = "${b}";
  $a = "${b[0]}";
  $a = "${b[ 0 ]}";
  $a = "${b['data']}";
  $a = "${ b }";
  $a = "${$b}";
  $a = "${b + 1}";
  $a = "${b.""}";
  $a = "${b && 1}";
  $a = "${b !== 1}";
  $a = "${b < 1}";
  $a = "${b ?? 1}";
  $a = "${b(print('foo'))}";
  $a = "${b[print('foo')]}";
  $a = "${b ? 1 : 2}";
  $a = "${$b}";

  // These are effectively parsed as referring to the constant `b` instead of
  // the variable `b`. If they were parsed as if they were referring to the
  // variable `b`, then only a single layer of subscripting would be permitted.
  $a = "${ b[0][1]}";
  $a = "${b [0][1]}";

  // Note: actually illegal in PHP (because member access is illegal on
  // constants), but caught by the typechecker. HHVM throws a runtime error.
  $a = "${b->c}";

  // Still interpolates `$b` and `$c`, but doesn't fail because of the `{$` in
  // the middle of the string.
  $a = "\{$b foo $c\}";

  $a = "{$b[1]}";
  $a = "{$b[1][2]}";
  $a = "{$b[1]()}";
  $a = "{$b[1]->c}";
  $a = "{$b[1]->c[2]}";
  $a = "{$b[print('hello')]}";
  $a = "{$b->c}";
  $a = "{$b[1]->c[2]}";
  $a = "{$b()}";
  $a = "{$b(c(1 + 2))}";
  $a = "{$b()[1]}";
  $a = "{$b()->c}";
  $a = "{$$b}";
}

function not_okay() {
  $a = "$b[";
  $a = "$b[]";
  $a = "$b[0";
  $a = "$b[ 0]";
  $a = "$b[0 ]";
  $a = "$b[$$c]";
  $a = "$b[$c->d]";
  $a = "$b[0\]";

  $a = "${b[0][1]}";
  $a = "${b = 1}";
  $a = "${b += 1}";

  $a = "{$b";
  $a = "{$b foo $c}";
  $a = "{$b is C}";
  $a = "{$b.$c}";
  $a = "{$b + 1}";
  $a = "{$b = 1}";
  $a = "{$b !== 1}";
  $a = "{$b ?? 1}";
  $a = "{$b[1] . 'c'}";
  $a = "{$b() . 'c'}";
  $a = "{$b::C}";
  $a = "{$b++}";
  $a = "{$b ? 1 : 2}";

  // Note: actually parses in PHP but appears to be mostly useless. At runtime,
  // looks up the static property literally named '$c'.
  $a = "{$b::$c}";
}

function implementation_defined_okay() {
  $a = "{$b?->c}";
  $a = "{$b->c()?->d}";
}
