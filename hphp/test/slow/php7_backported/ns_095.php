<?php

namespace Foo\Bar {
    class ClassA{}
    class ClassB{}
    class ClassC{}

    function fn_a(){ return __FUNCTION__; }
    function fn_b(){ return __FUNCTION__; }
    function fn_c(){ return __FUNCTION__; }

    const CONST_A = 1;
    const CONST_B = 2;
    const CONST_C = 3;
}

namespace Baz {

    use \Foo\Bar\{ClassA, ClassB, ClassC};
    use function \Foo\Bar\{fn_a, fn_b, fn_c};
    use const \Foo\Bar\{CONST_A, CONST_B, CONST_C};

    var_dump(ClassA::class);
    var_dump(ClassB::class);
    var_dump(ClassC::class);
    var_dump(fn_a());
    var_dump(fn_b());
    var_dump(fn_c());
    var_dump(CONST_A);
    var_dump(CONST_B);
    var_dump(CONST_C);

    echo "\nDone\n";
}
?>
