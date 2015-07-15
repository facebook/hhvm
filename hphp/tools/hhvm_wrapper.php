#!/usr/local/bin/php
<?hh
// Run with --help for help.
$FBCODE_ROOT = __DIR__.'/../..';
require_once $FBCODE_ROOT.'/hphp/tools/command_line_lib.php';

//////////////////////////////////////////////////////////////////////

function my_option_map(): OptionInfoMap {
  return Map {
'help'            => Pair { 'h', 'Print help message' },
'gdb'             => Pair { 'g', 'Run the whole command in gdb' },
'server'          => Pair { 's', 'Run a server, port 80, pwd as the root' },
'interp'          => Pair { 'i', 'Disable the JIT compiler' },
'opt-ir'          => Pair { 'o', 'Disable debug assertions in IR output' },
'dump-tc'         => Pair { 'j', 'Dump the contents of the translation cache' },
'dump-hhbc'       => Pair { 'v', 'Print the HHBC for units after compiling' },
'dump-hhas'       => Pair { '',  'Print the HHAS after compiling, then exit' },
'trace-printir::4'=> Pair { 'p', 'Extend TRACE with printir=n' },
'trace-hhir::3'   => Pair { 't', 'Extend TRACE with hhir=n' },
'hphpd'           => Pair { 'd', 'Run as the hphpd client' },
'compile'         => Pair { 'c', 'Compile with hphpc; run RepoAuthoritative' },
'create-repo'     => Pair { 'C', 'Compile unoptimized repo named hhvm.hhbc' },
'repo:'           => Pair { 'R', 'Use an already-compiled repo' },
'jit-gdb'         => Pair { '',  'Enable JIT symbols in GDB' },
'print-command'   => Pair { '',  'Just print the command, don\'t run it' },
'region-mode:'    => Pair { '',
                            'Which region selector to use (e.g \'method\')' },
'no-pgo'          => Pair { '',  'Disable PGO' },
'pgo-threshold:'  => Pair { '',  'PGO threshold to use' },
'no-obj-destruct' => Pair { '',
                            'Disable global object destructors in CLI mode' },
'zend'            => Pair { '',  'Enable ZendCompat functions and classes' },
'arm'             => Pair { '',  'Emit ARM code and simulate it' },
'ini[]'           => Pair { '',  'An .ini configuration file or CLI option' },
'hdf[]'           => Pair { '',  'An .hdf configuration file or CLI option' },
'no-defaults'     => Pair { '',
                            'Do not use the default wrapper runtime options'},
'build-root:'     => Pair { '',
                            'Override the default directory for hhvm and hphp'},
'perf:'           => Pair { '', 'Run perf record'},
  };
}

function get_paths(OptionMap $opts): Map<string,string> {
  $root = __DIR__.'/../../_bin/hphp';
  if (!is_dir($root)) {
    $root = __DIR__.'/..';
  }
  if ($opts->containsKey('build-root')) {
    $root = realpath($opts['build-root']);
  }
  return Map {
    'hhvm' => "$root/hhvm/hhvm",
    'hphp' => "$root/hhvm/hphp",
  };
}

function determine_flags(OptionMap $opts): string {
  $flags = '';
  $has_file = false;

  if ($opts->containsKey('ini')) {
    $ret = parse_config_options($opts['ini'], "ini");
    $flags .= $ret[0];
    $has_file = $ret[1];
  }
  if ($opts->containsKey('hdf')) {
    $ret = parse_config_options($opts['hdf'], "hdf");
    $flags .= $ret[0];
    // Don't override if we have an ini file already
    $has_file = $has_file || $ret[1];
  }
  // If no config files were given at the command line, use a default
  if (!$has_file) {
    #
    # The cli.hdf file is where Facebook puts its in-house
    # default configuration information.
    #
    $facebook_cli_config_file_name = '/usr/local/hphpi/cli.hdf';
    if (file_exists($facebook_cli_config_file_name)) {
      $flags .= "-c $facebook_cli_config_file_name ";
    }
  }

  if (!$opts->containsKey('no-defaults')) {
    $flags .=
      '-v Eval.EnableHipHopSyntax=true '.
      '-v Eval.EnableHipHopExperimentalSyntax=true '.
      '-v Eval.JitEnableRenameFunction=0 '.
      '-v Eval.GdbSyncChunks=1 '.
      '-v Eval.AllowHhas=true '.
      '';
  }

  if ($opts->containsKey('interp')) {
    $flags .=
      '-v Eval.Jit=0 '.
      '';
  } else {
    $flags .=
      '-v Eval.Jit=1 '.
      '';
  }

  if ($opts->containsKey('region-mode')) {
    if ($opts['region-mode'] == 'method') {
      $flags .=
        '-v Eval.JitLoops=1 '.
        '-v Eval.JitPGO=0 '.
        '';
      if (!$opts->containsKey('compile')) {
        echo 'Reminder: running region-mode=method without --compile is '.
             "almost never going to work...\n";
      }
    }
    if ($opts['region-mode'] == 'wholecfg') {
      $flags .=
        '-v Eval.JitPGORegionSelector='.((string)$opts['region-mode']).' '.
        '-v Eval.JitLoops=1 '.
        '';
    } else {
      $flags .=
        '-v Eval.JitRegionSelector='.((string)$opts['region-mode']).' '.
        '';
    }
  }

  $simple_args = Map {
    'dump-hhbc'       => '-v Eval.DumpBytecode=1 ',
    'dump-hhas'       => '-v Eval.DumpHhas=true ',
    'dump-tc'         => '-v Eval.DumpTC=1 ',
    'opt-ir'          => '-v Eval.HHIRGenerateAsserts=0 ',
    'jit-gdb'         => '-v Eval.JitNoGdb=false ',
    'no-pgo'          => '-v Eval.JitPGO=false ',
    'no-obj-destruct' => '-v Eval.EnableObjDestructCall=0 ',
    'zend'            => '-v Eval.EnableZendCompat=1 ',
    'hphpd'           => '-m debug ',
    'server'          => '-v Eval.JitPGOHotOnly=0 -m server ',
    'arm'             => '-v Eval.SimulateARM=1 ',
  };

  if ($opts->containsKey('pgo-threshold')) {
    $flags .=
      '-v Eval.JitPGOThreshold='.((string)$opts['pgo-threshold']).' '.
      '';
  }

  foreach ($simple_args as $k => $v) {
    if ($opts->containsKey($k)) {
      $flags .= $v;
    }
  }

  return $flags;
}

function parse_config_options(Set $options, string $kind): Pair<string, bool> {
  $flags = "";
  $has_file = false;
  foreach ($options as $option) {
    if (file_exists($option)) {
      $flags .= "-c " . $option . " ";
      $has_file = true;
    } else {
      $dashwhat = $kind === "ini" ? "-d " : "-v ";
      $flags .= $dashwhat . $option . " ";
    }
  }
  return Pair {$flags, $has_file};
}

function determine_env(OptionMap $opts): string {
  $trace_opts = Map {
    'trace-hhir'     => 'hhir',
    'trace-printir'  => 'printir',
  };
  $fixed_env = 'HPHP_TRACE_FILE=/dev/stdout HPHP_TRACE_TTY=1 ';

  $traces = Vector {};
  foreach ($trace_opts as $k => $v) {
    if ($opts->containsKey($k)) {
      $traces->add("$v:".((string)$opts[$k]));
    }
  }
  if ($traces->isEmpty()) return $fixed_env;

  $env = getenv("TRACE");
  if ($env) $env .= ',';
  return $fixed_env.
         'TRACE=' . $env . implode(',', $traces->toArray()) . ' ';
}

function argv_for_shell(): string {
  $ret = '';
  foreach ($GLOBALS['argv'] as $arg) {
    $ret .= '"'.$arg.'" ';
  }
  return $ret;
}

function compile_a_repo(bool $unoptimized, OptionMap $opts): string {
  $echo_command = $opts->containsKey('print-command');
  echo "Compiling with hphp...";

  $hphp_out='/tmp/hphp_out'.posix_getpid();
  $cmd = get_paths($opts)['hphp'].' '.
    '-v EnableHipHopSyntax=1 '.
    '-v EnableHipHopExperimentalSyntax=1 '.
    ($unoptimized ? '-v UseHHBBC=0 ' : '').
    '-t hhbc -k1 -l3 '.
    argv_for_shell().
    " >$hphp_out 2>&1";
  if ($echo_command) {
    echo "\n", $cmd, "\n";
  }
  system($cmd);
  echo "done.\n";

  $compile_dir = rtrim(shell_exec(
    'grep "all files saved in" '.$hphp_out.
    '| perl -pe \'s@.*(/tmp[^ ]*).*@$1@g\''
  ));
  $repo=$compile_dir.'/hhvm.hhbc';
  system("rm -f $hphp_out");
  if ($echo_command !== true) {
    register_shutdown_function(
      function() use ($compile_dir) {
        system("rm -fr $compile_dir");
      },
    );
  }

  return $repo;
}

function repo_auth_flags(string $flags, string $repo): string {
  return $flags .
    '-v Repo.Authoritative=true '.
    '-v Repo.Local.Mode=r- '.
    "-v Repo.Local.Path=$repo ".
    '--file ';
}

function compile_with_hphp(string $flags, OptionMap $opts): string {
  return repo_auth_flags($flags, compile_a_repo(false, $opts));
}

function create_repo(OptionMap $opts): void {
  $repo = compile_a_repo(true, $opts);
  system("cp $repo ./hhvm.hhbc");
}

function run_hhvm(OptionMap $opts): void {
  $flags = determine_flags($opts);
  if ($opts->containsKey('create-repo')) {
    create_repo($opts);
    exit(0);
  }
  if ($opts->containsKey('repo')) {
    $flags = repo_auth_flags($flags, (string) $opts['repo']);
  } else if ($opts->containsKey('compile')) {
    $flags = compile_with_hphp($flags, $opts);
  }

  $pfx = determine_env($opts);
  $pfx .= $opts->containsKey('gdb') ? 'gdb --args ' : '';
  if ($opts->containsKey('perf')) {
    $pfx .= 'perf record -g -o ' . $opts['perf'] . ' ';
  }
  $hhvm = get_paths($opts)['hhvm'];
  $cmd = "$pfx $hhvm $flags ".argv_for_shell();
  if ($opts->containsKey('print-command')) {
    echo "\n$cmd\n\n";
  } else {
    // Give the return value of the command back to the caller.
    $retval = null;
    passthru($cmd, $retval);
    exit($retval);
  }
}

function ends_with(string $haystack, string $needle): bool {
  return $needle === "" || substr($haystack, -strlen($needle)) === $needle;
}

function main(): void {
  $opts = parse_options(my_option_map());
  if ($opts->containsKey('help')) {
    help();
    return;
  }
  run_hhvm($opts);
}

//////////////////////////////////////////////////////////////////////

function help(): void {
  display_help(
"hhvm_wrapper\n".
"\n".
"   This script is a wrapper for hhvm and hphpc with command line\n".
"   flags that are a little more terse and geared toward common\n".
"   debugging/development tasks.  (Basically shorthands for various\n".
"   common combinations of RuntimeOption things.)\n".
"\n".
"   You might consider adding a bash alias or symlinking it to ~/bin.\n",
    my_option_map(),
  );

  echo
"Usage Examples:\n".
"\n".
"   # Run with the bytecode interpreter\n".
"   % hhvm -i test.php\n".
"\n".
"   # Run using the JIT:\n".
"   % hhvm test.php\n".
"\n".
"   # Dump the bytecode, then run:\n".
"   % hhvm -v test.php\n".
"\n".
"   # Compile repo with hphpc and run as repo authoritative, using the\n".
"   # JIT, with tracing.\n".
"   % TRACE=hhir:2 hhvm --compile -v test.php\n".
"\n".
"   # Same thing, but run it under gdb:\n".
"   % TRACE=hhir:2 hhvm -g --compile test.php\n".
"\n".
"   # Compile a repo, optimize it with hhbbc, then run using the output:\n".
"   % hhvm -C test.php  # creates hhvm.hhbc\n".
"   % hhbbc             # creates hhvm.hhbbc\n".
"   % hhvm --repo hhvm.hhbbc test.php\n".
"\n".
"   # Specify a configuration file to be used when running your code:\n".
"   % hhvm --ini test.ini test.php\n".
"\n".
"   # Specify multiple config files to be used when running your code:\n".
"   % hhvm --ini a.ini --hdf b.hdf --ini c.ini test.php\n".
"\n".
"   # Specify config option(s) to be used when running your code:\n".
"   % hhvm --ini hhvm.jit_a_size=15728640 test.php\n".
"\n"
    ;
}

//////////////////////////////////////////////////////////////////////

main();
