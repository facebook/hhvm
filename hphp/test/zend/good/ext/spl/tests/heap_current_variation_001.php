<?hh

class myHeap extends SplHeap
{
    public function compare($v1, $v2)
:mixed    {
        throw new Exception('');
    }
}
<<__EntryPoint>> function main(): void {
$heap = new myHeap();
var_dump($heap->current());
}
