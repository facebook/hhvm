<?hh

function nullable_str(): ?string {
  return null;
}

function demo(): void {
  $_ = HH\FIXME\UNSAFE_CAST<mixed, string>(nullable_str());
}
