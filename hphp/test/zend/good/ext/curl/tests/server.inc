<?hh

const PHP_CURL_SERVER_HOSTNAME = "localhost";
const PHP_CURL_SERVER_PORT = 8964;
const PHP_CURL_SERVER_ADDRESS =PHP_CURL_SERVER_HOSTNAME.":".PHP_CURL_SERVER_PORT;

function curl_cli_server_start() {
    if(getenv('PHP_CURL_HTTP_REMOTE_SERVER')) {
        return getenv('PHP_CURL_HTTP_REMOTE_SERVER');
    }

	$php_executable = getenv('TEST_PHP_EXECUTABLE');
	$doc_root = __DIR__;
	$router = "responder/get.inc";

	$descriptorspec = dict[
		0 => HH\stdin(),
		1 => HH\stdout(),
		2 => HH\stderr(),
	];

	if (substr(PHP_OS, 0, 3) == 'WIN') {
		$cmd = "{$php_executable} -t {$doc_root} -n -S " . PHP_CURL_SERVER_ADDRESS;
        $cmd .= " {$router}";
		$handle = proc_open(addslashes($cmd), $descriptorspec, $pipes, $doc_root, NULL, dict["bypass_shell" => true,  "suppress_errors" => true]);
	} else {
		$php_executable__str = (string)($php_executable);
		$cmd = "exec {$php_executable__str} -t {$doc_root} -n -S " . (string)(PHP_CURL_SERVER_ADDRESS);
		$cmd .= " {$router}";
		$cmd .= " 2>/dev/null";

                $pipes = null;
		$handle = proc_open($cmd, $descriptorspec, inout $pipes, $doc_root);
	}

	// note: even when server prints 'Listening on localhost:8964...Press Ctrl-C to quit.'
	//       it might not be listening yet...need to wait until fsockopen() call returns
    $i = 0;
    $errno = null;
    $errstr = null;
    $error = error_reporting();
    error_reporting(0);
    while (($i++ < 30) &&
           !($fp = fsockopen(
               PHP_CURL_SERVER_HOSTNAME,
               PHP_CURL_SERVER_PORT,
               inout $errno,
               inout $errstr,
             ))
          ) {
        usleep(10000);
    }
    error_reporting($error);

    if ($fp) {
        fclose($fp);
    }

	register_shutdown_function(
		function() use ($handle) {
			proc_terminate($handle);
		}
	);
	// don't bother sleeping, server is already up
	// server can take a variable amount of time to be up, so just sleeping a guessed amount of time
	// does not work. this is why tests sometimes pass and sometimes fail. to get a reliable pass
	// sleeping doesn't work.
    return PHP_CURL_SERVER_ADDRESS;
}
