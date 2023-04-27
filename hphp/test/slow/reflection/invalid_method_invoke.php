<?hh
<<__EntryPoint>> function main(): void {
$rc = new ReflectionClass('XSLTProcessor');
echo $rc->getMethod('setSecurityPrefs')->invoke(new XMLReader(), 0xdeadbeef);
}
