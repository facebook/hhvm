<?php

class a {
	function a(){
	}
}
class b {
}

$b = new b();

$a=new ReflectionClass("a");
$m=$a->getMethod("a");

try {
        $m->invoke(null);
} catch (ReflectionException $E) {
        echo $E->getMessage()."\n";
}


try {
        $m->invoke($b);
} catch (ReflectionException $E) {
        echo $E->getMessage()."\n";
}

$b = new a();
try {
        $m->invoke($b);
} catch (ReflectionException $E) {
        echo $E->getMessage()."\n";
}

echo "===DONE===\n";?>
