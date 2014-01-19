<?php

require __DIR__."/random_port.inc";

function http_server_skipif($socket_string) {

	if (!function_exists('pcntl_fork')) die('skip pcntl_fork() not available');
	if (!function_exists('posix_kill')) die('skip posix_kill() not available');
	if (!stream_socket_server($socket_string)) die('skip stream_socket_server() failed');
}

/* Minimal HTTP server with predefined responses.
 *
 * $socket_string is the socket to create and listen on (e.g. tcp://127.0.0.1:1234)
 * $files is an array of files containing N responses for N expected requests. Server dies after N requests.
 * $output is a stream on which everything sent by clients is written to
 */
function http_server($socket_string, array $files, &$output = null)
{
	ini_set('default_socket_timeout', 5);
	pcntl_alarm(10);

	$server = stream_socket_server($socket_string, $errno, $errstr);
	if (!$server) {
		return false;
	}

	if ($output === null) {
		if (false===($output = tmpfile())) {
			return false;
		}
	}

	$pid = pcntl_fork();
	if ($pid == -1) {
		die('could not fork');
	} else if ($pid) {
		return $pid;
	}

	foreach($files as $file) {
		$sock = stream_socket_accept($server);
		if (!$sock) {
			exit(1);
		}

		// read headers

		$content_length = 0;

		while (false!==($line=trim(fgets($sock)))) {
			fwrite($output, "$line\r\n");
			if (''===$line) {
				break;
			} else {
				if (preg_match('#^Content-Length:\s*([[:digit:]]+)\s*$#i', $line, $matches)) {
					$content_length = (int) $matches[1];
				}
			}
		}

		// read content
		if ($content_length > 0) {
			$line = fread($sock, $content_length);
			fwrite($output, "$line\r\n");
		}

		// send response

		fputs($sock, $file);
		fclose($sock);
	}

	exit(0);
}

function http_server_kill($pid) {
	posix_kill($pid, SIGTERM);
	pcntl_waitpid($pid, $status);
}

?>
