<?php

echo "-TEST\n";

class a implements Serializable {
    function serialize() {
        return serialize(get_object_vars($this));
    }
    function unserialize($s) {
        foreach (unserialize($s) as $p=>$v) {
            $this->$p=$v;
        }
    }
}
class b extends a {}
class c extends b {}

$c = new c;
$c->a = new a;
$c->a->b = new b;
$c->a->b->c = $c;
$c->a->c = $c;
$c->a->b->a = $c->a;
$c->a->a = $c->a;

$s = serialize($c);
printf("%s\n", $s);

$d = unserialize($s);

var_dump(
    $d === $d->a->b->c,
    $d->a->a === $d->a,
    $d->a->b->a === $d->a,
    $d->a->c === $d
);

print_r($d);

echo "Done\n";

?>