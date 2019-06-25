<?hh <<__EntryPoint>> function main(): void {
$data = file(__FILE__);
try {
  foreach ($data as $line) {
    throw new Exception("error");
  }
} catch (Exception $e) {
  echo "ok\n";
}
}
