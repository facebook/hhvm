<?hh
class Test implements Iterator {

    public $arr = varray[];

    public function rewind()    { $arr = $this->arr; reset(inout $arr); $this->arr = $arr;  }
    public function current()   { throw new Exception(); }
    public function key()       { $arr = $this->arr; $key = key($arr); $this->arr = $arr; return $key; }
    public function next()      { $arr = $this->arr; $n = next(inout $arr); $this->arr = $arr; return $n; }
    public function valid()     { $arr = $this->arr; $x = current($arr); $this->arr = $arr; return $x !== false; }
}
<<__EntryPoint>> function main(): void {
$t = new Test();
$t->arr =  varray[1, 2, 3];

try {
    foreach ($t as $v) {
        echo "$v\n";
    }
} catch (Exception $e) {
    ; // handle exception
}
echo "ok\n";
}
