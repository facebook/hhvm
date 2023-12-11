<?hh

class A {
    public $test = false;
    public $test2 = '';
    public $test3 = vec[];
    public $_ = false;
}


<<__EntryPoint>>
function main_mb_convert_variables_empty() :mixed{
$a = new A();
mb_convert_variables('utf-8', 'windows-1251', inout $a);
var_dump($a);

$a = dict['test' => vec[]];
mb_convert_variables('utf-8', 'windows-1251', inout $a);
var_dump($a);
}
