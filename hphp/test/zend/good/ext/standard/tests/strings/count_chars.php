<?hh <<__EntryPoint>> function main(): void {
$s = "het leven is net erwtensoep - je kunt er geen touw aan vastknopen";
for($i=0; $i<3; $i++) {
    echo implode(count_chars($s, $i))."\n";
}
$a = count_chars($s, 3);
echo $a, "\n";
echo (int) strlen(count_chars($s, 4)) == 256-strlen($a),"\n";
}
