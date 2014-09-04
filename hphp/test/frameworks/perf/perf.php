<?hh

require_once('HHVMDaemon.php');
require_once('HHVMStats.php');
require_once('PHP5Daemon.php');
require_once('NginxDaemon.php');
require_once('Siege.php');
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
  PerfTarget $target,
  PHPEngine $php_engine,
  string $temp_dir,
) {
  print_progress('Installing framework');
  $target->install();

  print_progress('Starting Nginx');
  $nginx = new NginxDaemon($temp_dir, $target);
  $nginx->start();
  assert($nginx->isRunning());

  print_progress('Starting PHP Engine');
  $php_engine->start();
  assert($php_engine->isRunning());

  if ($target->needsUnfreeze()) {
    print_progress('Unfreezing framework');
    $target->unfreeze();
  }

  print_progress('Starting Siege for warmup');
  $siege = new Siege($temp_dir, $target, RequestModes::WARMUP);
  $siege->start();
  assert($siege->isRunning());
  $siege->wait();

  assert(!$siege->isRunning());
  assert($php_engine->isRunning());

  print_progress('Waiting 30s for server to stabilize');
  sleep(30);

  print_progress('Enabling engine stats collection');
  $php_engine->enableStats();

  print_progress('Running Siege for benchmark');
  $siege = new Siege($temp_dir, $target, RequestModes::BENCHMARK);
  $siege->start();
  assert($siege->isRunning());
  $siege->wait();

  print_progress('Collecting results');
  $php_engine_stats = $php_engine->collectStats();
  $siege_stats = $siege->collectStats();

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

  print(json_encode($combined_stats, JSON_PRETTY_PRINT)."\n");

  print_progress('All done');
  $php_engine->stop();
}

function perf_main($argv) {
  $options = getopt(
    '',
    [
      'php5:', 'hhvm:',
      'toys', 'wordpress',
      'help'
    ]
  );
  if (array_key_exists('help', $options)) {
    fprintf(
      STDERR,
      "Usage: %s --<php5=/path/to/php-cgi|hhvm=/path/to/hhvm> ".
      "--<toys|wordpress>\n",
      $argv[0],
    );
    exit(0);
  }

  $temp_dir = tempnam(sys_get_temp_dir(), 'hhvm-nginx');
  // Currently a file - change to a dir
  unlink($temp_dir);
  mkdir($temp_dir);

  $target = null;
  $engine = null;

  if (array_key_exists('wordpress', $options)) {
    $target = new WordpressTarget($temp_dir);
  }
  if (array_key_exists('toys', $options)) {
    $target = new ToysTarget();
  }
  if ($target === null) {
    throw new Exception('Either --wordpress or --toys must be specified');
  }

  if (array_key_exists('php5', $options)) {
    $engine = new PHP5Daemon($temp_dir, $target, $options['php5']);
  }
  if (array_key_exists('hhvm', $options)) {
    $engine = new HHVMDaemon($temp_dir, $target, $options['hhvm']);
  }
  if ($engine === null) {
    throw new Exception(
      'Either --php5=/path/to/php-cgi or --hhvm=/path/to/hhvm '.
      'must be specified'
    );
  }

  run_benchmark($target, $engine, $temp_dir);
}

perf_main($argv);
