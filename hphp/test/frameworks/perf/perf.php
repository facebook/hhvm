<?hh

require_once('HHVMDaemon.php');
require_once('HHVMStats.php');
require_once('PHP5Daemon.php');
require_once('NginxDaemon.php');
require_once('PerfOptions.php');
require_once('Siege.php');
require_once('SugarCRMTarget.php');
require_once('ToysTarget.php');
require_once('WordpressTarget.php');

function print_progress(string $out): void {
  $timestamp = strftime('%Y-%m-%d %H:%M:%S');
  $len = max(strlen($out), strlen($timestamp));
  fprintf(
    STDERR,
    "\n%s\n** %s\n** %s\n",
    str_repeat('*', $len + 3), // +3 for '** '
    $timestamp,
    $out,
  );
}

function run_benchmark(
  PerfOptions $options,
  PerfTarget $target,
  PHPEngine $php_engine
) {
  print_progress('Installing framework');
  $target->install();

  print_progress('Starting Nginx');
  $nginx = new NginxDaemon($options, $target);
  $nginx->start();
  Process::sleepSeconds($options->delayNginxStartup);
  invariant($nginx->isRunning(), 'Failed to start nginx');

  print_progress('Starting PHP Engine');
  $php_engine->start();
  Process::sleepSeconds($options->delayPhpStartup);
  invariant(
    $php_engine->isRunning(),
    'Failed to start '.get_class($php_engine)
  );

  if ($target->needsUnfreeze()) {
    print_progress('Unfreezing framework');
    $target->unfreeze($options);
  }

  if ($options->skipSanityCheck) {
    print_progress('Skipping sanity check');
  } else {
    print_progress('Running sanity check');
    $target->sanityCheck();
  }

  print_progress('Starting Siege for warmup');
  $siege = new Siege($options, $target, RequestModes::WARMUP);
  $siege->start();
  invariant($siege->isRunning(), 'Failed to start siege');
  $siege->wait();

  invariant(!$siege->isRunning(), 'Siege is still running :/');
  invariant($php_engine->isRunning(), get_class($php_engine).' crashed');

  print_progress('Enabling engine stats collection');
  $php_engine->enableStats();

  print_progress('Clearing nginx access.log');
  $nginx->clearAccessLog();

  print_progress('Running Siege for benchmark');
  $siege = new Siege($options, $target, RequestModes::BENCHMARK);
  $siege->start();
  invariant($siege->isRunning(), 'Siege failed to start');
  $siege->wait();

  print_progress('Collecting results');
  $php_engine_stats = $php_engine->collectStats();
  $siege_stats = $siege->collectStats();
  $nginx_stats = $nginx->collectStats();

  $combined_stats = Map { };
  foreach ($php_engine_stats as $page => $stats) {
    $combined_stats[$page] = $stats;
  }
  foreach ($siege_stats as $page => $stats) {
    if ($combined_stats->containsKey($page)) {
      $combined_stats[$page]->setAll($stats);
    } else {
      $combined_stats[$page] = $stats;
    }
  }
  foreach ($nginx_stats as $page => $stats) {
    if ($combined_stats->containsKey($page)) {
      $combined_stats[$page]->setAll($stats);
    } else {
      $combined_stats[$page] = $stats;
    }
  }

  if (!$options->verbose) {
    $combined_stats = $combined_stats->filterWithKey(
      ($k, $v) ==> $k === 'Combined'
    );
  } else {
    ksort($combined_stats);
  }
  print(json_encode($combined_stats, JSON_PRETTY_PRINT)."\n");

  print_progress('All done');
  $php_engine->stop();
}

function perf_main($argv) {
  PerfSettings::Validate();
  if (getmyuid() === 0) {
    fwrite(STDERR, "Run this script as a regular user.\n");
    exit(1);
  }

  $options = new PerfOptions();

  if ($options->help) {
    fprintf(
      STDERR,
      "Usage: %s --<php5=/path/to/php-cgi|hhvm=/path/to/hhvm> ".
      "--<toys|wordpress|sugarcrm-login-page>\n",
      $argv[0],
    );
    exit(0);
  }

  // If we exit cleanly, Process::__destruct() gets called, but it doesn't
  // if we're killed by Ctrl-C. This tends to leak php-cgi or hhvm processes -
  // trap the signal so we can clean them up.
  pcntl_signal(
    SIGINT,
    function() {
      Process::cleanupAll();
      exit();
    }
  );

  $target = null;
  $engine = null;

  if ($options->wordpress) {
    $target = new WordpressTarget($options);
  }
  if ($options->toys) {
    $target = new ToysTarget();
  }
  if ($options->sugarcrm) {
    $target = new SugarCRMTarget($options);
  }
  if ($target === null) {
    fprintf(
      STDERR,
      "You must specify a target with one of the following:\n".
      "  --toys\n".
      "  --wordpress\n".
      "  --sugarcrm-login-page\n"
    );
    exit(1);
  }

  if ($options->php5) {
    $engine = new PHP5Daemon($options, $target);
  }
  if ($options->hhvm) {
    $engine = new HHVMDaemon($options, $target);
  }
  if ($engine === null) {
    fprintf(
      STDERR,
      'Either --php5=/path/to/php-cgi or --hhvm=/path/to/hhvm '.
      'must be specified'
    );
    exit(1);
  }

  run_benchmark($options, $target, $engine);
}

perf_main($argv);
