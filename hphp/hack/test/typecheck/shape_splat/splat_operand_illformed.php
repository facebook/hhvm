<?hh

<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// The operand of a shape splat is recursively checked, so an ill-formed hint
// inside a splat is still reported: well-formedness (`?void`, `tuple<>`) via
// typing_type_wellformedness, and instantiability (a trait) via the
// instantiability TAST check. (Unbound names / wildcards are caught earlier in
// naming.)

trait MyTrait {}

type CTrait = shape(...MyTrait, 'a' => int);
type COptVoid = shape(...?void, 'b' => int);
type CTuple = shape(...Tuple<int, string>, 'c' => int);
