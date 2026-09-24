<?hh

function extra_arguments(): vec<string> {
  echo "unpacked argument evaluated\n";
  return vec[];
}

<<__EntryPoint>>
function main(): void {
  HH\classname_to_class_strict_isolation_backdoor(
    "C",
    ...extra_arguments()
  );
}
