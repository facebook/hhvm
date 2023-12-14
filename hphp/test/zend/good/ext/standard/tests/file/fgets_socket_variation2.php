<?hh
const LINE_OF_DATA = "12345678\n";

// create a file
<<__EntryPoint>> function main(): void {
$filename = sys_get_temp_dir().'/'.'fgets_socket_variation2.tmp';
$fd = fopen($filename, "w+");

// populate the file with lines of data
for ($i = 0; $i < 1000; $i++) {
    fwrite($fd, LINE_OF_DATA);
}
fclose($fd);

for ($i=0; $i<100; $i++) {
  $port = rand(10000, 65000);
  /* Setup socket server */
  $errno = null;
  $errstr = null;
  $server = @stream_socket_server(
    "tcp://127.0.0.1:$port",
    inout $errno,
    inout $errstr
  );
  if ($server) {
    break;
  }
}

/* Connect to it */
$client = fsockopen("tcp://127.0.0.1:$port", -1, inout $errno, inout $errstr);

if (!$client) {
    exit("Unable to create socket");
}

/* Accept that connection */
$peername = null;
$socket = stream_socket_accept($server, -1.0, inout $peername);

echo "Write data from the file:\n";
$data = file_get_contents($filename);
unlink($filename);

var_dump(fwrite($socket, $data));
fclose($socket);

echo "\nRead lines from the client\n";
while ($line = fgets($client,256)) {
    if (strcmp($line, LINE_OF_DATA) != 0) {
        echo "Error - $line does not match " . LINE_OF_DATA;
        break;
    }
}

echo "\nClose the server side socket and read the remaining data from the client\n";
fclose($server);
while(!feof($client)) {
    fread($client, 1);
}

echo "done\n";
}
