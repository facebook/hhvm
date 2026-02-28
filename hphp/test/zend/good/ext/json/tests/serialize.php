<?hh

class NonSerializingTest
{
    public $data;

    public function __construct($data)
    {
        $this->data = $data;
    }
}

class SerializingTest extends NonSerializingTest implements JsonSerializable
{
    public function jsonSerialize()
:mixed    {
        return $this->data;
    }
}

class ValueSerializingTest extends SerializingTest
{
    public function jsonSerialize()
:mixed    {
        return array_values(is_array($this->data) ? $this->data : get_object_vars($this->data));
    }
}

class SelfSerializingTest extends SerializingTest
{
    public function jsonSerialize()
:mixed    {
        return $this;
    }
}
<<__EntryPoint>> function main(): void {
$adata = dict[
    'str'    => 'foo',
    'int'    => 1,
    'float'    => 2.3,
    'bool'    => false,
    'nil'    => null,
    'arr'    => vec[1,2,3],
    'obj'    => new stdClass,
];

$ndata = array_values($adata);
$odata = new stdClass();
$odata->str = 'foo';
$odata->int = 1;
$odata->float = 2.3;
$odata->bool = false;
$odata->nil = null;
$odata->arr = vec[1,2,3];
$odata->obj = new stdClass();

foreach(vec['NonSerializingTest','SerializingTest','ValueSerializingTest','SelfSerializingTest'] as $class) {
    echo "==$class==\n";
    echo json_encode(new $class($adata)), "\n";
    echo json_encode(new $class($ndata)), "\n";
    echo json_encode(new $class($odata)), "\n";
}
}
