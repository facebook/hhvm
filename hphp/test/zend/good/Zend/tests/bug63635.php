<?hh
class Node {
    public $parent = NULL;
    public $childs = vec[];

    function __construct(?Node $parent=NULL) {
        if ($parent) {
            $parent->childs[] = $this;
        }
        $this->childs[] = $this;
    }
}

const MAX = 16;
<<__EntryPoint>> function main(): void {
for ($n = 0; $n < 20; $n++) {
    $top = new Node();
    for ($i=0 ; $i<MAX ; $i++) {
        $ci = new Node($top);
        for ($j=0 ; $j<MAX ; $j++) {
            $cj = new Node($ci);
            for ($k=0 ; $k<MAX ; $k++) {
                $ck = new Node($cj);
            }
        }
    }
    echo "$n\n";
}
echo "ok\n";
}
