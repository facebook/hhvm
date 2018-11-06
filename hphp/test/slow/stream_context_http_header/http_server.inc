<?php

/**
 * @param string $socketString the socket to create and listen on (e.g. tcp://127.0.0.1:1234)
 * @param string[] $responses an array of N responses for N expected requests. Server dies after N requests.
 * @param resource $output a stream on which everything sent by clients is written to
 * @return int pid
 */
function httpServer($socketString, array $responses, &$output = null)
{
    ini_set('default_socket_timeout', 5);
    pcntl_alarm(10);

    $server = stream_socket_server($socketString, $errorNumber, $errorString);
    if (!$server) {
        echo "stream_socket_server() returned: ($errorNumber) \"$errorString\"";
        exit(1);
    }

    if ($output === null) {
        if (false === ($output = tmpfile())) {
            echo "tmpfile() failed.";
            exit(1);
        }
    }

    $pid = pcntl_fork();
    if ($pid == -1) {
        echo "pcntl_fork() failed.";
        exit(1);
    } elseif ($pid) {
        return $pid;
    }

    foreach ($responses as $key => $file) {
        $sock = stream_socket_accept($server);
        if (!$sock) {
            echo "stream_socket_accept() failed.";
            exit(1);
        }

        // read request-line and headers
        $contentLength = 0;
        while (false !== ($line = trim(fgets($sock)))) {
            fwrite($output, "$line\r\n");

            if ('' === $line) {
                break;
            }

            if (preg_match('#^Content-Length:\s*([[:digit:]]+)\s*$#i', $line, $matches)) {
                $contentLength = (int) $matches[1];
            }
        }

        // read message-body
        if ($contentLength > 0) {
            $line = fread($sock, $contentLength);
            fwrite($output, "$line\r\n");
        }

        // send response
        fputs($sock, $file);
        fclose($sock);
    }
    return null;
}

function httpServerKill($pid)
{
    posix_kill($pid, SIGTERM);
    pcntl_waitpid($pid, $status);
}

function httpServerSkipIf($socketString)
{
    if (!function_exists('pcntl_fork')) die('skip pcntl_fork() not available');
    if (!function_exists('posix_kill')) die('skip posix_kill() not available');
    if (!stream_socket_server($socketString)) die('skip stream_socket_server() failed');
}
