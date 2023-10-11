<?hh

function null_<T>(): ?T {
  return null;
}

function test(): nonnull {
  return null_();
}
