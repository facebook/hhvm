<?hh
function f($x) :mixed{
    echo "f($x) ";
    return $x;
}
<<__EntryPoint>> function main(): void {
echo "Function call args:\n";
$i=0; $__l = f($i); ++$i; $__r = f($i); var_dump($__l < $__r);
$i=0; $__l = f($i); ++$i; $__r = f($i); var_dump($__l <= $__r);
$i=0; $__l = f($i); ++$i; $__r = f($i); var_dump($__l > $__r);
$i=0; $__l = f($i); ++$i; $__r = f($i); var_dump($__l >= $__r);

echo "\nArray indices:\n";
$a = dict[1 => dict[], 3 => dict[]];
$a[1][2] = 0;
$a[3][4] = 1;
$i=0;
$i=1; $__a=$i; ++$i; $__b=$i; $__l=$a[$__a][$__b]; ++$i; $__c=$i; ++$i; $__d=$i; $__r=$a[$__c][$__d]; var_dump($__l < $__r);
$i=1; $__a=$i; ++$i; $__b=$i; $__l=$a[$__a][$__b]; ++$i; $__c=$i; ++$i; $__d=$i; $__r=$a[$__c][$__d]; var_dump($__l <= $__r);
$i=1; $__a=$i; ++$i; $__b=$i; $__l=$a[$__a][$__b]; ++$i; $__c=$i; ++$i; $__d=$i; $__r=$a[$__c][$__d]; var_dump($__l > $__r);
$i=1; $__a=$i; ++$i; $__b=$i; $__l=$a[$__a][$__b]; ++$i; $__c=$i; ++$i; $__d=$i; $__r=$a[$__c][$__d]; var_dump($__l >= $__r);
}
