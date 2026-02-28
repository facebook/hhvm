<?hh

function test($nix, AnyArray $ar, inout $ref, stdClass $std, NonExistingClass $na, inout stdClass $opt, $def = "FooBar")
:mixed{
}

class test
{
    function test($nix, AnyArray $ar, $ref, stdClass $std, NonExistingClass $na, ?stdClass $opt = NULL, $def = "FooBar")
:mixed    {
    }
}

function check_params_decl_func($r, $f)
:mixed{
    $c = $r->$f();
    echo $f . ': ' . ($c ? ($c is ReflectionMethod ? $c->class . '::' : '') . $c->name : 'NULL') . "()\n";
}

function check_params_decl_class($r, $f)
:mixed{
    $c = $r->$f();
    echo $f . ': ' . ($c ? $c->name : 'NULL') . "\n";
}

function check_params_func($r, $f)
:mixed{
    echo $f . ': ';
    $v = $r->$f();
    var_dump($v);
}

function check_params($r)
:mixed{
    echo "#####" . ($r is ReflectionMethod ? $r->class . '::' : '') . $r->name . "()#####\n";
    $i = 0;
    foreach($r->getParameters() as $p)
    {
        echo "===" . $i . "===\n";
        $i++;
        check_params_func($p, 'getName');
        check_params_func($p, 'isPassedByReference');
        try
        {
            check_params_decl_class($p, 'getClass');
        }
        catch(ReflectionException $e)
        {
            echo $e->getMessage() . "\n";
        }
        check_params_decl_class($p, 'getDeclaringClass');
//        check_params_decl_func($p, 'getDeclaringFunction');
        check_params_func($p, 'isArray');
        check_params_func($p, 'allowsNull');
        check_params_func($p, 'isOptional');
        check_params_func($p, 'isDefaultValueAvailable');
        if ($p->isOptional())
        {
            check_params_func($p, 'getDefaultValue');
        }
    }
}
<<__EntryPoint>> function main(): void {
check_params(new ReflectionFunction('test'));

check_params(new ReflectionMethod('test::test'));

echo "===DONE===\n";
}
