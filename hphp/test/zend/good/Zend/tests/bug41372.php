<?hh <<__EntryPoint>> function main(): void {
$Foo = array('val1', 'val2', 'val3');
end(inout $Foo);
echo key($Foo),"\n";
$MagicInternalPointerResetter = $Foo;
echo key($Foo),"\n";
}
