<?hh
class A {
    public $e = varray[];
}

class Token implements \Serializable {
    public function serialize()
    {
        $c = new A;

        for ($i = 0; $i < 4; $i++)
        {
            $e = new A;
            $c->e[] = $e;
            $e->e = $c->e;
        }

        return serialize(varray[serialize($c)]);
    }

    public function unserialize($str)
    {
        $r = unserialize($str);
        $r = unserialize($r[0]);
    }
}
<<__EntryPoint>> function main(): void {
echo "Test\n";

$token = new Token;
$token = serialize($token);
echo "Done";
}
