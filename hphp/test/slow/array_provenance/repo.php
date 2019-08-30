<?hh

class ConstBag {
  const string MY_TRASH_GARBAGE = 'crap';
}

<<__EntryPoint>>
function main() {
  $a = rand();

  $b = __hhvm_intrinsics\launder_value(
    dict[ConstBag::MY_TRASH_GARBAGE => $a]
  );
  var_dump(HH\get_provenance($b));
}
