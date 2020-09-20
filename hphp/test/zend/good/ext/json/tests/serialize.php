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
    {
        return $this->data;
    }
}

class ValueSerializingTest extends SerializingTest
{
    public function jsonSerialize()
    {
        return array_values(is_array($this->data) ? $this->data : get_object_vars($this->data));
    }
}

class SelfSerializingTest extends SerializingTest
{
    public function jsonSerialize()
    {
        return $this;
    }
}
<<__EntryPoint>> function main(): void {
$adata = darray[
    'str'    => 'foo',
    'int'    => 1,
    'float'    => 2.3,
    'bool'    => false,
    'nil'    => null,
    'arr'    => varray[1,2,3],
    'obj'    => new StdClass,
];

$ndata = array_values($adata);
$odata = new stdClass();
$odata->str = 'foo';
$odata->int = 1;
$odata->float = 2.3;
$odata->bool = false;
$odata->nil = null;
$odata->arr = varray[1,2,3];
$odata->obj = new stdClass();

foreach(varray['NonSerializingTest','SerializingTest','ValueSerializingTest','SelfSerializingTest'] as $class) {
    echo "==$class==\n";
    echo json_encode(new $class($adata)), "\n";
    echo json_encode(new $class($ndata)), "\n";
    echo json_encode(new $class($odata)), "\n";
}
}
