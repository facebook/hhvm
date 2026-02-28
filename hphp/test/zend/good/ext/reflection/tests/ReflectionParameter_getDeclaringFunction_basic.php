<?hh
function ReflectionParameterTest($test, $test2 = null) :mixed{
    echo $test;
}
<<__EntryPoint>> function main(): void {
$reflect = new ReflectionFunction('ReflectionParameterTest');
$params = $reflect->getParameters();
foreach($params as $key => $value) {
    echo $value->getDeclaringFunction() . "\n";
}
echo "==DONE==";
}
