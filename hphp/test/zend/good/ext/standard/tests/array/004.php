<?hh <<__EntryPoint>> function main(): void {
$data = array(
    'Test1',
    'teST2'=>0,
    5=>'test2',
    'abc'=>'test10',
    'test21'
);

var_dump($data);

natsort(inout $data);
var_dump($data);

natcasesort(inout $data);
var_dump($data);
}
