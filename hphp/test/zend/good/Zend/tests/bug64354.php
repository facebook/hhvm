<?hh
class B implements Serializable {
    public function serialize() :mixed{
        throw new Exception("serialize");
        return NULL;
    }

    public function unserialize($data) :mixed{
    }
}
<<__EntryPoint>> function main(): void {
$data = vec[new B];

try {
    serialize($data);
} catch (Exception $e) {
    var_dump($e->getMessage());
}
}
