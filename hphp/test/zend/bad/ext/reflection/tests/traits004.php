<?php
trait T1 { }
trait T2 { }

class C1 { }
class C2 { use T1; }
class C3 { use T1; use T2; }

for ($c = "C1"; $c <= "C3"; $c++) {
    echo "class $c:\n";
    $r = new ReflectionClass($c);
    var_dump($r->getTraitNames());
    var_dump($r->getTraits());
    echo "\n";
}
