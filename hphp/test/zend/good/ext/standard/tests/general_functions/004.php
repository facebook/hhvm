<?hh <<__EntryPoint>> function main(): void {
chdir(dirname(__FILE__));
$fp=fopen("004.data","r");
$a=fgetcsv($fp,100,"\t");
while($a) {
    echo join(",",$a)."\n";
    $a=fgetcsv($fp,100,"\t");
}
fclose($fp);
}
