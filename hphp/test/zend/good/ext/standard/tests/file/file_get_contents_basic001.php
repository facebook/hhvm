<?hh
<<__EntryPoint>> function main(): void {
$file_content = "Bienvenue au CodeFest a Montreal";
$temp_filename = sys_get_temp_dir().'/'.'fichier_a_lire.txt';
$handle = fopen($temp_filename,"w");
fwrite($handle,$file_content);
fclose($handle);
$var = file_get_contents($temp_filename);
echo $var;

unlink($temp_filename);
}
