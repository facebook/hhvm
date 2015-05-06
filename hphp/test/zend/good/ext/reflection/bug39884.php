<?php
class stubParamTest
{
    function paramTest(self $param)
    {
        // nothing to do
    }
}
$test1 = new stubParamTest();
$test2 = new stubParamTest();
$test1->paramTest($test2);
$refParam = new ReflectionParameter(array('stubParamTest', 'paramTest'), 'param');
var_dump($refParam->getClass());
?>
