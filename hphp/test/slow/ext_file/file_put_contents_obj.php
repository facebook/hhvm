<?hh

class foo {
    public function __toString()
    {
        return 'hello';
    }
}


<<__EntryPoint>>
function main_file_put_contents_obj() {
$tmpfname = tempnam("/tmp", "FOO");
$object = new foo;
file_put_contents($tmpfname, $object);
echo file_get_contents($tmpfname);
}
