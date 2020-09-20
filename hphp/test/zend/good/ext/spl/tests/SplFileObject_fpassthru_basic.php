<?hh <<__EntryPoint>> function main(): void {
$obj = new SplFileObject(dirname(__FILE__).'/SplFileObject_testinput.csv');
$obj->fpassthru();
}
