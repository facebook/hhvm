<?hh
class EnglishMealIterator implements Iterator {
    private $pos=0;
    private $myContent=vec["breakfast", "dinner", "tea"];

    public function valid() :mixed{
        echo "--> " . __METHOD__ . " ($this->pos)\n";
        return $this->pos<3;
    }

    public function next() :mixed{
        echo "--> " . __METHOD__ . " ($this->pos)\n";
        return $this->myContent[$this->pos++];
    }

    public function rewind() :mixed{
        echo "--> " . __METHOD__ . " ($this->pos)\n";
        $this->pos=0;
    }

    public function current() :mixed{
        echo "--> " . __METHOD__ . " ($this->pos)\n";
        return $this->myContent[$this->pos];
    }

    public function key() :mixed{
        echo "--> " . __METHOD__ . " ($this->pos)\n";
        return "meal " . $this->pos;
    }

}

class A1 implements IteratorAggregate {
    function getIterator() :mixed{
        return new EnglishMealIterator;
    }
}

class A2 implements IteratorAggregate {
    function getIterator() :mixed{
        return new A1;
    }
}

class A3 implements IteratorAggregate {
    function getIterator() :mixed{
        return new A2;
    }
}
<<__EntryPoint>> function main(): void {
echo "\n-----( A1: )-----\n";
foreach (new A1 as $k=>$v) {
    echo "$k => $v\n";
}

echo "\n-----( A2: )-----\n";
foreach (new A2 as $k=>$v) {
    echo "$k => $v\n";
}

echo "\n-----( A3: )-----\n";
foreach (new A3 as $k=>$v) {
    echo "$k => $v\n";
}

echo "===DONE===\n";
}
