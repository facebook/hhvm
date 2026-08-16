<?hh

<<__EntryPoint>>
function main(): void {
  // STDIN is fd0, so if FDs are re-used in lightprocess/cli-server/...,
  // it's the most at-risk.
  $spec = dict[
    1 => tuple('pipe', 'w'),
    2 => tuple('pipe', 'w'),
    9 => tuple('pipe', 'w'),
  ];
  $pipes = dict[];
  $proc = proc_open('echo FOO; echo BAR >&2; echo BAZ>&9', $spec, inout $pipes);
  var_dump(fread($pipes[1], 1024));
  var_dump(fread($pipes[2], 1024));
  var_dump(fread($pipes[9], 1024));
}
