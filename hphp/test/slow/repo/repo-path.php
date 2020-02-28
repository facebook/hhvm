<?hh

<<__EntryPoint>>
function main() {
  $argc = $GLOBALS['argc'];
  $argv = $GLOBALS['argv'];

  if ($argc > 1) {
    echo getenv('SUDO_USER') ?: getenv('USER');
    exit;
  }

  $repo_prefix = tempnam('/tmp', 'repo-path-');
  if (!$repo_prefix) return;
  $repo_prefix .= '-verify_';

  $repo_pattern = $repo_prefix.'%{user}.hhbc';

  $cmd = implode(' ', varray[
    PHP_BINARY,
    '-vRepo.Local.Mode=--',
    '-vRepo.Central.Path='.$repo_pattern,
    __FILE__,
    'dummy'
  ]);

  $descriptorspec = varray[
     varray['pipe', 'r'],
     varray['pipe', 'w'],
     varray['pipe', 'w']
  ];

  $pipes = null;
  $proc = proc_open($cmd, $descriptorspec, inout $pipes);
  if (!$proc) {
    echo "Failed to open: $cmd\n";
    exit -1;
  }
  $user = trim(stream_get_contents($pipes[1]));

  $repo_path = $repo_prefix.$user.'.hhbc';

  fclose($pipes[1]);
  proc_close($proc);

  if (file_exists($repo_path)) {
    echo 'success';
    unlink($repo_path);
  } else {
    echo 'failure';
  }
}
