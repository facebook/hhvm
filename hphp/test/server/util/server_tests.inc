<?hh


abstract final class ServerUtilServerTests {
  public static $error_log_file;
  public static $test_run_id = 0;
  public static $LOG_ROOT;
  public static $DOC_ROOT;
  public static $next_instance_id = 0;
  public static $repo_builds = dict[];
}

ServerUtilServerTests::$test_run_id = posix_getpid();
ServerUtilServerTests::$error_log_file = fopen(ServerUtilServerTests::$LOG_ROOT.'_test'.ServerUtilServerTests::$test_run_id.".log", 'w');

function tlog($str) {


  fwrite(ServerUtilServerTests::$error_log_file, $str ?? '');
  fwrite(ServerUtilServerTests::$error_log_file, "\n");
  fflush(ServerUtilServerTests::$error_log_file);
  // error_log($str);
}

function dumpLogFilesToStdoutAndDie() {
  $LOG_ROOT = ServerUtilServerTests::$LOG_ROOT;
  $test_run_id = ServerUtilServerTests::$test_run_id;

  sleep(1);
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_test$test_run_id.log'");
  readfile("${LOG_ROOT}_test$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_test_server$test_run_id.log'");
  readfile("${LOG_ROOT}_test_server$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_test_server_stdout$test_run_id.log'");
  readfile("${LOG_ROOT}_test_server_stdout$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_test_server_stderr$test_run_id.log'");
  readfile("${LOG_ROOT}_test_server_stderr$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_test_client$test_run_id.log'");
  readfile("${LOG_ROOT}_test_client$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_test_sandbox_access.log'");
  readfile("${LOG_ROOT}_test_sandbox_access.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_curl$test_run_id.log'");
  readfile("${LOG_ROOT}_curl$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  throw new Exception("test failed");
}

function hphp_home() {
  // __DIR__ == result.'hphp/test/server/util'
  return realpath(__DIR__.'/../../../..');
}

function get_random_port($exclude1, $exclude2) {
  $BasePort = 20000;
  $PortRange = 3000;
  do {
    $t = rand($BasePort, $BasePort+$PortRange);
  } while ($t == $exclude1 || $t == $exclude2);
  return $t;
}

# Return the command line to start the server with the specified options
function getServerCmd($serverPort, $adminPort, $debugPort, $home, $root,
                      $customArgs = '', $serverId = null, $repoArgs = '') {
  $LOG_ROOT = ServerUtilServerTests::$LOG_ROOT;
  $test_run_id = ServerUtilServerTests::$test_run_id;

  $instance_id = ServerUtilServerTests::$next_instance_id;
  ++ServerUtilServerTests::$next_instance_id;

  $portConfig = ' -vServer.Port='.$serverPort;
  $serverConfig = ' --config='.$home.'/config/server.ini';
  $logFileConfig = ' -vLog.File='."${LOG_ROOT}_test_server$test_run_id.log";
  $logFileConfig.= ' -vLog.Access.Default.File='.
    "${LOG_ROOT}_access_log$test_run_id.log";
  $srcRootConfig = ' -vServer.SourceRoot='.$root;
  $includePathConfig = ' -vServer.IncludeSearchPaths.0='.$root;
  $adminPortConfig = $adminPort ? ' -vAdminServer.Port='.$adminPort : '';
  $debugPortConfig = $debugPort ? ' -vEval.Debugger.Port='.$debugPort : '';
  $repoConfig = " -vRepo.Central.Path=${LOG_ROOT}_server{$test_run_id}_{$instance_id}.hhbc";
  $useJit = array_key_exists('HHVM_JIT', $_ENV) && $_ENV['HHVM_JIT'] == 1;
  $jitConfig = ' -vEval.Jit='.($useJit ? "true" : "false");
  // To emulate sandbox setup, let Sandbox.Home be '$home'
  // and user name be 'debugger', so that the server can find the
  // sandbox_conf.hdf in '$root'.
  $sandboxHomeConfig = ' -vSandbox.Home='.$home;
  $serverId ??= $test_run_id;

  $hhvm = PHP_BINARY;

  if (ini_get('hhvm.repo.authoritative')) {
    if (count(ServerUtilServerTests::$repo_builds) === 0) {
      register_shutdown_function(
         function() {
           foreach (ServerUtilServerTests::$repo_builds as $dir) {
             @unlink($dir . "/hhvm.hhbc");
             @rmdir($dir);
           }
         }
       );
    }
    $repoKey = 'x'.$root.':'.$repoArgs;
    if (!isset(ServerUtilServerTests::$repo_builds[$repoKey])) {
      ServerUtilServerTests::$repo_builds[$repoKey] =
        "${LOG_ROOT}_bytecode{$test_run_id}_{$instance_id}";
      $cmd = "$hhvm --hphp -k1 -l2 -t hhbc -o " . ServerUtilServerTests::$repo_builds[$repoKey] .
        " --input-dir $root $repoArgs";

      tlog('Building repo with command: '.$cmd);
      tlog(exec($cmd));
    }
    $repoConfig .=
      " -vRepo.Local.Path=".ServerUtilServerTests::$repo_builds[$repoKey]."/hhvm.hhbc".
      " -vRepo.Authoritative=true";
  }

  $cmd = "exec env MALLOC_CONF=junk:true TESTID={$serverId} " .
    "SERVERPORT={$serverPort} $hhvm" .
    ' --mode=server' . $serverConfig . $logFileConfig .
    ' -vServer.ExitOnBindFail=true' .
    ' --instance-id=' . $test_run_id .
    ' -vPageletServer.ThreadCount=5' .
    $portConfig . $srcRootConfig .
    $includePathConfig . $sandboxHomeConfig . $adminPortConfig .
    $debugPortConfig . $repoConfig . $jitConfig . ' ' . $customArgs .
    " > ${LOG_ROOT}_test_server_stdout$test_run_id.log" .
    " 2> ${LOG_ROOT}_test_server_stderr$test_run_id.log";

  return $cmd;
}

function startServer(&$serverPort, &$adminPort, &$debugPort, $home, $root,
                     $customArgs = '', $serverId = null, $repoArgs = '') {


  $chooseServer = $serverPort === null;
  $chooseAdmin = $adminPort === null;
  $chooseDebug = $debugPort === null;
  $pid = posix_getpid();
  $safe_children = array_flip(array_map(
    $v ==> (int)$v,
    explode(',', exec("pgrep -f -d , -P $pid")),
  ));

  for ($i = 0; $i < 5; $i++) {
    if ($chooseServer) $serverPort = get_random_port($adminPort, $debugPort);
    if ($chooseAdmin) $adminPort = get_random_port($serverPort, $debugPort);
    if ($chooseDebug) $debugPort = get_random_port($serverPort, $adminPort);

    $cmd = getServerCmd($serverPort, $adminPort, $debugPort, $home, $root,
                        $customArgs, $serverId, $repoArgs);
    tlog('Starting server with command: '.$cmd);
    $pipes = array();
    $serverProc = proc_open($cmd, array(), &$pipes);
    if (!is_resource($serverProc)) {
      tlog('Failed to start a shell process for the server');
    } else if (waitForServerToGetGoing($serverPort, $serverProc, $serverId)) {
      return $serverProc;
    }
    killChildren($pid, $safe_children);
    if ($serverProc) proc_close($serverProc);
  }
  dumpLogFilesToStdoutAndDie();
}

// Check if the server id is in the expected list of ids.
function checkServerId($serverPort, $expectedIds) {
  $host = php_uname('n');
  $r = request($host, $serverPort, "hello.php");
  if (preg_match('/Hello, World!(.*+)/', $r ?? '', &$matches)) {
    foreach ((array)$expectedIds as $id) {
      if ($matches[1] == $id) return true;
    }
    tlog('a server for a different test responded');
    return false;
  }
  tlog('Server replied: '.$r);
  return false;
}

function waitForServerToGetGoing($serverPort, $serverProc, $serverId = null) {

  $serverId ??= ServerUtilServerTests::$test_run_id;
  for ($i = 1; $i <= 20; $i++) {
    $status = proc_get_status($serverProc);
    if ($status === false || !$status['running']) {
      break;
    }
    sleep(1);
    if (checkServerId($serverPort, $serverId)) {
      return true;
    }
  }

  tlog('Server is not responding.');
  return false;
}

function stopServer($adminPort, $serverProc) {
  $LOG_ROOT = ServerUtilServerTests::$LOG_ROOT;
  $test_run_id = ServerUtilServerTests::$test_run_id;


  $r = "";
  for ($i = 1; $i <= 10; $i++) {
    $r = request(php_uname('n'), $adminPort, 'stop?instance-id='.$test_run_id);
    if ($r === "OK") break;
    usleep(100000);
  }
  if ($r != "OK") {
    tlog("Server did not stop. Response was $r");
    dumpLogFilesToStdoutAndDie();
  }
  proc_close($serverProc);

  @unlink("${LOG_ROOT}_test$test_run_id.log");
  @unlink("${LOG_ROOT}_test_server$test_run_id.log");
  @unlink("${LOG_ROOT}_test_server_stderr$test_run_id.log");
  @unlink("${LOG_ROOT}_test_server_stdout$test_run_id.log");
  @unlink("${LOG_ROOT}_test_client$test_run_id.log");
  @unlink("${LOG_ROOT}_client$test_run_id.hhbc");
  @unlink("${LOG_ROOT}_curl$test_run_id.log");
  @unlink("${LOG_ROOT}_access_log$test_run_id.log");

  $next_instance_id = ServerUtilServerTests::$next_instance_id;
  for ($instance_id = 0; $instance_id < $next_instance_id; $instance_id++) {
    @unlink("${LOG_ROOT}_server{$test_run_id}_{$instance_id}.hhbc");
  }
}

// Start a new server to takeover the old one, we want to assign a new
// server id in order to distinguish between the two servers.
function takeoverOldServer($serverPort, $adminPort, $home, $root,
                           $socketFile, $oldServerProc, $customArgs,
                           $serverId) {
  $status = proc_get_status($oldServerProc);
  if ($status === false || !$status['running']) {
    tlog('Old server is not running');
    return;
  }
  $customArgs .= " -vServer.TakeoverFilename=${socketFile}";
  $cmd = getServerCmd($serverPort, $adminPort, false, $home, $root,
                      $customArgs, $serverId);
  $pipes = array();
  $serverProc = proc_open($cmd, array(), &$pipes);
  if (!is_resource($serverProc)) {
    tlog('Failed to start a shell process for the server');
    return;
  }
  return $serverProc;
}

function http_request($host, $port, $path, $timeout = 1200, $curl_opts = '') {

  if (is_array($path)) {
    $headers = $path[2] ?? null;
    $post = $path[1] ?? null;
    $path = $path[0];
  } else {
    $headers = null;
    $post = null;
  }
  if ($post) {
    $post = http_build_query($post);
    $post = '--data-urlencode='.urlencode($post);;
  }
  if ($headers) {
    $s = array();
    foreach ($headers as $h => $v) {
      $s[] = "-H '$h: $v'";
    }
    $headers = implode(" ", $s);
  }
  $url = "http://$host:$port/$path";
  $host_name = "hphpd.debugger.".php_uname('n');

  $LOG_ROOT = ServerUtilServerTests::$LOG_ROOT;
  $test_run_id = ServerUtilServerTests::$test_run_id;
  $cmd = "curl $post --trace-ascii ${LOG_ROOT}_curl$test_run_id.log ".
    "--silent $curl_opts --connect-timeout $timeout ".
    "-H 'Host: $host_name' $headers --url \"$url\"";
  tlog("Requesting page with command: $cmd");
  if (exec($cmd, &$result) === null) return null;
  return implode("\n", $result);
}

function requestAll(array $requests, $customArgs = '', $repoArgs = '') {
  runTest(
    function($serverPort) use ($requests) {
      foreach ($requests as $request) {
        $r = is_array($request) ? $request[0] : $request;
        echo "Requesting '$r'\n";
        var_dump(request(php_uname('n'), $serverPort, $request));
      }
    },
    $customArgs,
    $repoArgs,
  );
}

if (!function_exists("request")) {
  include 'server_tests1.inc';
}

function killChildren($pid, $safe_children = array()) {
  $childIds = exec("pgrep -f -d , -P $pid");
  foreach (array_map($v ==> (int)$v, explode(",", $childIds ?? '')) as $cid) {
    if (!$cid) continue;
    if (isset($safe_children[$cid])) continue;
    tlog("killing ".exec("ps -f -p ".$cid));
    killChildren($cid, $safe_children);
    posix_kill($cid, SIGKILL);
  }
}
