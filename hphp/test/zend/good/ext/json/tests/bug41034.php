<?hh <<__EntryPoint>> function main(): void {
echo json_encode(darray[0 => 0, "\0ab" => 1, 1 => "\0null-prefixed value"]);
echo "\nDone\n";
}
