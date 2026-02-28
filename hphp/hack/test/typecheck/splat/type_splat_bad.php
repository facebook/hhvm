<?hh



// Illegal because splat is followed by another parameter (two errors)
function bad1<Targs as (mixed...)>(
  (function(...Targs, int): void) $f,
  ... Targs $params,
  int $arg,
): void {
}

// Illegal because splat is followed by a variadic (two errors)
function bad2<Targs as (mixed...)>(
  (function(...Targs, int...): void) $f,
  ... Targs $params,
  int ...$args
): void {
}

// Illegal because splat is followed by an optional (two errors)
function bad3<Targs as (mixed...)>(
  (function(...Targs, optional int): void) $f,
  ... Targs $params,
  int $arg = 3,
): void {
}

// Illegal because splat is preceded by a variadic (two errors)
function bad5<Targs as (mixed...)>(
  (function(int..., ...Targs): void) $f,
  int ...$args,
  ... Targs $params,
): void {
}

// Illegal because splat has default value (one error)
function bad6<Targs super (int, int, mixed...)>(
  ... Targs $params = tuple(1, 2),
): void {
}

interface I {
  // Illegal because splat is optional (two errors)
  public function bad7<Targs as (mixed...)>(
    (function(int, optional ...Targs): void) $f,
    optional ... Targs $params,
  ): void;
  // Illegal because splat is variadic (two errors)
  public function bad8<Targs as (mixed...)>(
    (function(int, ...Targs...): void) $f,
    ... Targs ...$params
  ): void;
}

// Illegal because splat has inout (two errors)
function bad9<Targs as (mixed...)>(
  (function(int, inout ...Targs): void) $f,
  inout ... Targs $args,
): void {}

// Illegal because splat has readonly (two errors)
function bad10<Targs as (mixed...)>(
  (function(int, readonly ...Targs): void) $f,
  readonly ... Targs $args,
): void {}
