<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function direct<TData as HH\Runtime\RepresentableAs<dict<arraykey, mixed>>>(TData $assoc): mixed {
  return HH\Runtime\reveal($assoc)['time'];
}
