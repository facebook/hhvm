<?hh <<__EntryPoint>> function main(): void {
$x = new ReflectionMethod('ReflectionParameter', 'export');
$y = function() { };

try {
    $x->invokeArgs(new ReflectionParameter('trim', 'str'), vec[$y, 1]);
} catch (Exception $e) { }
echo "ok";
}
