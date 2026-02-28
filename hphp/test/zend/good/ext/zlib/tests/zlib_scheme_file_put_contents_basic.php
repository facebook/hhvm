<?hh
<<__EntryPoint>> function main(): void {
$outputFileName = sys_get_temp_dir().'/'.'tmp';
$outFile = "compress.zlib://$outputFileName";
$data = <<<EOT
Here is some plain
text to be read
and displayed.
EOT;

file_put_contents($outFile, $data);
$h = gzopen($outputFileName, 'r');
gzpassthru($h);
gzclose($h);
echo "\n";
unlink($outputFileName);
echo "===DONE===\n";
}
