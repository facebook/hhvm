//// file1.php
<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// A newtype is transparent inside its defining file and opaque outside it, where
// it behaves exactly like a rigid type parameter: an opaque spread element whose
// field interval comes from the type it is declared `as`.

newtype NTBound as shape('a' => int, ...) = shape('a' => int, 'secret' => bool);

newtype NTPlain = shape('a' => int);

// Transparent here, so both expand to their definitions and merge away.
function inside_bound(shape(...NTBound) $s): void {
  hh_expect_equivalent<shape('a' => int, 'secret' => bool)>($s);
}

function inside_plain(shape(...NTPlain) $s): void {
  hh_expect_equivalent<shape('a' => int)>($s);
}

//// file2.php
<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Opaque here: the element survives normalization rather than being expanded,
// and a splat of it alone is just the newtype.
function outside_bound(shape(...NTBound) $s): void {
  hh_expect_equivalent<NTBound>($s);
}

// In a row with a concrete element it stays as a residual element.
function outside_row(shape(...NTBound, 'x' => int) $s): void {
  hh_expect_equivalent<shape(...NTBound, 'x' => int)>($s);
}

// The `as` bound supplies the field interval, so 'a' is readable...
function read_bound(shape(...NTBound, 'x' => int) $s): void {
  hh_expect<int>($s['a']);
}

// ...but the definition is not visible, so 'secret' is not.
function read_secret(shape(...NTBound, 'x' => int) $s): void {
  hh_expect<nothing>($s['secret']);
}

// REJECT: no `as` clause, so the bound is `mixed`, which cannot be spread.
function outside_plain(shape(...NTPlain) $s): void {}

// A type parameter bounded BY a newtype is spreadable exactly when the newtype
// is, so the bound is followed transitively.
function tparam_bounded_by_newtype<T as NTBound>(shape(...T) $s): void {
  hh_expect<int>($s['a']);
}

// REJECT: transitively, `NTPlain`'s bound is `mixed`.
function tparam_bounded_by_plain<T as NTPlain>(shape(...T) $s): void {}
