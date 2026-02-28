<?hh

function f_with_args(
  int $i,
  string $name = "DEFAULT",
  bool $enabled = false,
) {
  if ($enabled) {
    echo "Enabled: $i $name";
  } else {
    echo "Disabled: $i $name";
  }
}
