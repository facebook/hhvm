<?hh

class test extends PDO
{
    protected function isProtected() :mixed{
        echo "this is a protected method.\n";
    }
    private function isPrivate() :mixed{
        echo "this is a private method.\n";
    }

    public function quote($str, $paramtype = NULL) :mixed{
        $this->isProtected();
        $this->isPrivate();
        print $str ."\n";
    }
}
<<__EntryPoint>> function main(): void {
$test = new test('sqlite::memory:');
$test->quote('foo');
$test->isProtected();
}
