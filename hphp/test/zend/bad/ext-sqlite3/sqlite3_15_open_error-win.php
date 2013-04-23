<?php
$sysroot = exec('echo %systemroot%');
$icacls = "$sysroot\\System32\\icacls.exe";
$user = get_current_user();
$unreadable = __DIR__ . '/unreadable.db';

touch($unreadable);
$cmd = $icacls . ' ' . $unreadable . ' /inheritance:r /deny ' . $user . ':(F,M,R,RX,W)';
exec($cmd);

try {
	$db = new SQLite3($unreadable);
} catch (Exception $e) {
	echo $e . "\n";
}
echo "Done\n";

$cmd = $icacls . ' ' . $unreadable . ' /grant ' . $user . ':(F,M,R,RX,W)';
exec($cmd);
unlink($unreadable);
?>