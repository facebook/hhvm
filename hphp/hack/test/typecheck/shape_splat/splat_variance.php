<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Type parameters in splat position can only ever be data so we should reject
// type parameters declared as contravariant

type NotReallyContrav<-T> = shape(...T, 'x' => int);

type ReallyCov<+T> = shape(...T, 'x' => int);

type OkAsInv<T> = shape(...T, 'x' => int);
