<?hh



abstract class Box {
  abstract const type T;
}

function getter<T>(Box with { type T = T } $box) : T {
  while(true) {}
}

function f<T as Box with { type T = int }>(T $genbox) : int {
  // Here we check that subtyping works correctly on queries
  // of the form Tgeneric(_) <: Thas_type_member(_)
  return getter($genbox);
}
