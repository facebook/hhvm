<?hh <<__EntryPoint>> function main() {
try {
    $msg = "Some error \x00 message";
    throw new Exception($msg);
} catch(Exception $e) {
    var_dump($e->getMessage(), $msg);
}
}
