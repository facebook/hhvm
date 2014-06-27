<?php
class a{}
class b extends a{}
class c extends b{}
class d{}
var_dump(class_parents(new c),
         class_parents("c"),
         class_parents(new b),
         class_parents("b"),
         class_parents("d"),
         class_parents("foo", 0),
         class_parents("foo", 1)
);

interface iface1{}
interface iface2{}
class f implements iface1, iface2{}
var_dump(class_implements(new a),
         class_implements("a"),
         class_implements("aaa"),
         class_implements("bbb", 0)
);

function __autoload($cname) {
    var_dump($cname);
}

?>
===DONE===
<?php exit(0); ?>