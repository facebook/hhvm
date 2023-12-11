<?hh
class A {
    public $e = vec[];
}

class Token implements \Serializable {
    public function serialize()
:mixed    {
        $c = new A;

        for ($i = 0; $i < 4; $i++)
        {
            $e = new A;
            $c->e[] = $e;
            $e->e = $c->e;
        }

        return serialize(vec[serialize($c)]);
    }

    public function unserialize($str)
:mixed    {
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
