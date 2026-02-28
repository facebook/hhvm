<?hh <<__EntryPoint>> function main(): void {
$trans = dict["a" => 1,
               "b" => 1,
               "c" => 2,
               "z" => 0,
               "d" => TRUE,
               "E" => FALSE,
               "F" => NULL,
               0 => "G",
               1 => "h",
               2 => "i"];
$trans = array_flip($trans);
var_dump($trans);
}
