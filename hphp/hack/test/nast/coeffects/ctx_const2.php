<?hh

<<file: __EnableUnstableFeatures('union_intersection_types')>>

class CWithConst {
  const ctx C = [io, rand];
  const type C1 = (\HH\Capabilities\IO & \HH\Capabilities\Rand);
}
