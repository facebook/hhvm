<?hh

class foo {
    public function __toString()
:mixed    {
        return 'hello';
    }
}


<<__EntryPoint>>
function main_file_put_contents_obj() :mixed{
$tmpfname = tempnam(sys_get_temp_dir(), "FOO");
$object = new foo;
file_put_contents($tmpfname, $object);
echo file_get_contents($tmpfname);
}
