<?hh

function disallowed_case_aliases(
  Int $_,
  Bool $_,
  Float $_,
  String $_,
  DArray $_,
  VArray $_,
  Varray_or_darray $_,
  Vec_or_dict $_,
  This $_,
  ): void {}


function allowed_case_aliases(
  Null $_,
  Void $_,
  Num $_,
  Resource $_,
  Arraykey $_,
  Mixed $_,
  Nonnull $_,
  Nothing $_,
  ): void {}
