<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type CT1 as shape(?'a' => nonnull) = shape(?'a' => CT1);
case type CT2 as shape(?'a' => shape(?'a' => nonnull)) = shape(?'a' => CT2);
