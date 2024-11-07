<?hh

interface I {
  public function __toString(): mixed;
}

<<__EntryPoint>>
function main(): void {
  $li = I::class;
  $li_launder = __hhvm_intrinsics\launder_value($li);

  $i = HH\classname_to_class($li);
  $i_launder = HH\classname_to_class($li_launder);
  var_dump(is_a($i, nameof StringishObject, true));
  var_dump(is_a($i_launder, nameof StringishObject, true));
}
