<?hh

class �berK��li��
{
	public $��������ber = '������';
}

<<__EntryPoint>> function main(): void {
$������ = darray['������' => '������'];

$foo = new �berk��li��();

var_dump(serialize($foo));
var_dump(unserialize(serialize($foo)));
var_dump(serialize($������));
var_dump(unserialize(serialize($������)));
}
