<?php
require 'server.inc';

$file = "mediumfile.txt";

$ftp = ftp_connect('127.0.0.1', $port);
ftp_login($ftp, 'user', 'pass');
if (!$ftp) die("Couldn't connect to the server");

$local_file = dirname(__FILE__) . DIRECTORY_SEPARATOR . $file;
touch($local_file);

$r = ftp_nb_get($ftp, $local_file, $file, FTP_BINARY);
while ($r == FTP_MOREDATA) {
    $r = ftp_nb_continue($ftp);
}
ftp_close($ftp);

echo file_get_contents($local_file);
?>
<?php
@unlink(dirname(__FILE__) . DIRECTORY_SEPARATOR . "mediumfile.txt");
?>