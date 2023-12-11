<?hh
class A {
    public function __sleep() :mixed{
        throw new Exception("Failed");
    }
}

class B implements Serializable {
    public function serialize() :mixed{
        return NULL;
    }

    public function unserialize($data) :mixed{
    }
}
<<__EntryPoint>> function main(): void {
$data = vec[new A, new B];

try {
    serialize($data);
} catch (Exception $e) {
    var_dump($e->getMessage());
}
}
