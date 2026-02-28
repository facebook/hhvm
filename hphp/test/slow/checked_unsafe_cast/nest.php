<?hh

function nn(): <<__Soft>> nonnull {
  return HH\FIXME\UNSAFE_NONNULL_CAST(
    HH\FIXME\UNSAFE_NONNULL_CAST(
      null
    )
  );
}
function nu(): <<__Soft>> nonnull {
  return HH\FIXME\UNSAFE_NONNULL_CAST(
    HH\FIXME\UNSAFE_CAST<mixed, nonnull>(
      null
    )
  );
}
function un(): <<__Soft>> nonnull {
  return HH\FIXME\UNSAFE_CAST<mixed, nonnull>(
    HH\FIXME\UNSAFE_NONNULL_CAST(
      null
    )
  );
}
function uu(): <<__Soft>> nonnull {
  return HH\FIXME\UNSAFE_CAST<mixed, nonnull>(
    HH\FIXME\UNSAFE_CAST<mixed, nonnull>(
      null
    )
  );
}

<<__EntryPoint>>
function main(): void {
  nn();
  nu();
  un();
  uu();
}
