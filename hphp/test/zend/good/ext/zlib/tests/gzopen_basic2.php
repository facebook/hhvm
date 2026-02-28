<?hh
/* Prototype  : resource gzopen(string filename, string mode [, int use_include_path])
 * Description: Open a .gz-file and return a .gz-file pointer 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing gzopen() : basic functionality ***\n";


// Initialise all required variables
$filename = sys_get_temp_dir().'/'."gzopen_basic2.txt.gz";
$modes = vec['w', 'w+'];
$data = "This was the information that was written";

foreach($modes as $mode) {
   echo "testing mode -- $mode --\n";
   $h = gzopen($filename, $mode);
   if ($h !== false) {
      gzwrite($h, $data);
      gzclose($h);
      $h = gzopen($filename, 'r');
      gzpassthru($h);
      gzclose($h);
      echo "\n";
      unlink($filename);
   }
   else {
      var_dump($h);
   }
}      
      
echo "===DONE===\n";
}
