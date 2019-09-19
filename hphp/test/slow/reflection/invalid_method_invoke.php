<?hh
<<__EntryPoint>> function main(): void {
$rc = new ReflectionClass('XsltProcessor');
echo $rc->getMethod('setSecurityPrefs')->invoke(new XmlReader(), 0xdeadbeef);
}
