<?hh <<__EntryPoint>> function main(): void {
$data = '(#11/19/2002#)';
var_dump(preg_split('/\b/', $data));
}
