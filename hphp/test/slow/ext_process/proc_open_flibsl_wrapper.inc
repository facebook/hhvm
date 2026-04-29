<?hh

namespace FlibSL\PHP;

function proc_open(
  string $cmd,
  darray<int, vec<string>> $descriptorspec,
  inout mixed $pipes,
): mixed {
  return \proc_open($cmd, $descriptorspec, inout $pipes);
}
