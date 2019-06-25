<?hh
<<__EntryPoint>> function main(): void {
$file_content = "Bienvenue au CodeFest a Montreal";
$temp_filename = dirname(__FILE__)."/fichier_a_lire.txt";
$handle = fopen($temp_filename,"w");
fwrite($handle,$file_content);
fclose($handle);
$var = file_get_contents($temp_filename);
echo $var;
error_reporting(0);
$temp_filename = dirname(__FILE__)."/fichier_a_lire.txt";
unlink($temp_filename);
}
