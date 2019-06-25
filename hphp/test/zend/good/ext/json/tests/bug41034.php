<?hh <<__EntryPoint>> function main(): void {
echo json_encode(array(0,"\0ab"=>1,"\0null-prefixed value"));
echo "\nDone\n";
}
