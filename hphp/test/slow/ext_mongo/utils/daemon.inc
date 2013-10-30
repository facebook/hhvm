<?php
include dirname(__FILE__) . "/cfg.inc";

function d($msg) {
    if (DEBUG) {
        echo trim($msg), "\n";
    }
}


$socket = stream_socket_server("tcp://0.0.0.0:8000", $errno, $errstr);
if (!$socket) {
    echo "$errstr ($errno)\n";
    exit(1);
}

$descriptorspec = array(
    0 => array("pipe", "r+"),
    1 => array("pipe", "w"),
    2 => array("file", "/tmp/error-output.txt", "a"),
);

if (!file_exists($SHELL)) {
    throw new Exception("I cannot find '$SHELL', did you set the \$SHELL varaible correctly?");
}
$process = proc_open("$SHELL $SHELL_PARAMS", $descriptorspec, $IO, dirname($SHELL));
if (!$process) {
    echo "Can't execute '$SHELL $SHELL_PARAMS'\n";
    exit(2);
}

stream_set_write_buffer($IO[0], 0);
/* Added in 5.3.3 */
if (function_exists("stream_set_read_buffer")) {
    stream_set_read_buffer($IO[1], 0);
}

// Say hi to the shell and read it back so we know everything is working
$cmd = "print(" . json_encode($MARKER) . ")\n\n\n\n";
d($cmd);
fwrite($IO[0], $cmd);
fflush($IO[0]);
$c = 0;
do {
    $w = $e = null;
    $r = array($IO[1]);
    if (!stream_select($r, $w, $e, 10)) {
        echo "Can't read the hello world from the shell.. timed out\n";
        exit(3);
    }

    $line = fgets($r[0]);
    if (trim($line) == $MARKER) {
        d("Read marker successfully");
        break;
    }
    d("Got '" . trim($line). "' from shell");
    if ($c > 100) {
        echo "Bailing out, can't seem to be able to make sense of the shell!\n";
        printf("Does '%s' with the arguments '%s' make sense?\n", $SHELL, $SHELL_PARAMS);
        exit(3);
    }
} while(1);


d("Now accepting connection..");
do {
    while ($conn = @stream_socket_accept($socket)) {
        d("Accepted a connection..");
        /* Empty any data we have from the shell, if any, its unrelated */
        $r = array($IO[1]);
        $w = $e = null;
        if (stream_select($r, $w, $e, 0, 1000)) {
            d("Discarding stuff from shell");
            $cmd = "print(" . json_encode($MARKER) . ")\n\n\n\n";
            d($cmd);
            fwrite($IO[0], $cmd);
            fflush($IO[0]);
            while(true) {
                $l = fgets($r[0]);
                if (trim($l) == $MARKER) {
                    break;
                }
            }
        }
        $badread = 0;
        do {
            $r = array($IO[1], $conn);
            $w = $e = null;
            if (stream_select($r, $w, $e, 1)) {
                if ($r) {
                    foreach($r as $reader) {
                        if ($reader === $IO[1]) {
                            d("Can read from shell");
                            $line = fgets($reader);
                            d("Read '$line'");
                            $success = fwrite($conn, $line);
                            if (!$success) {
                                break 2;
                            }
                            fflush($conn);
                        }
                        if ($reader === $conn) {
                            //d("Can read from connection");
                            $buf = trim(fgets($reader));
                            if ($buf == $QUIT) {
                                fwrite($conn, "I resign!\n");
                                break 4;
                            }
                            if ($buf == "quit") {
                                break 2;
                            }
                            d($buf);
                            //d("Read '$buf'");
                            // Sometimes we for some reason don't notice the 
                            // connection is dead
                            if ($buf == "") {
                                if ($badread++ > 3) {
                                    break 2;
                                }
                            } else {
                                $badread = 0;
                            }
                            // Trimming the buffer and appending ;\n to it is 
                            // important for the shell for some reason
                            fwrite($IO[0],  $buf . ";\n");
                            fflush($IO[0]);
                        }
                    }
                }
                //d("Read what I could");
            } else {
                d("select() timed out");
                if (feof($conn)) {
                    break;
                }
            }
        } while(1);

        d("Terminating connection..");
        fclose($conn);
    }
} while(true);
echo "Closing stuff!\n";

// Make sure we have shutdown all we want
$cmd = sprintf("shutdownEverything(function(){ print(%s); });\n", json_encode($MARKER));
d($cmd);
fwrite($IO[0], $cmd);
fflush($IO[0]);
$c = 0;
do {
    $w = $e = null;
    $r = array($IO[1]);
    if (!stream_select($r, $w, $e, 10)) {
        echo "Can't read the hello world from the shell.. timed out\n";
        exit(3);
    }

    $line = fgets($r[0]);
    if (trim($line) == $MARKER) {
        d("Read marker successfully");
        break;
    }
    d("Got '" . trim($line). "' from shell");
    if ($c > 100) {
        echo "Bailing out, can't seem to be able to make sense of the shell!\n";
        printf("Does '%s' with the arguments '%s' make sense?\n", $SHELL, $SHELL_PARAMS);
        exit(3);
    }
} while(1);

fclose($IO[0]);
fclose($IO[1]);
fclose($socket);
proc_close($process);
fclose($conn);
