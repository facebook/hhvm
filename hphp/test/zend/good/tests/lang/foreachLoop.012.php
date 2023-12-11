<?hh

const MAX_LOOPS = 5;

function withRefValue($elements, $transform) :mixed{
    echo "\n---( Array with $elements element(s): )---\n";
    //Build array:
    $a = dict[];
    for ($i=0; $i<$elements; $i++) {
        $a[] = "v.$i";
    }
    $counter=0;

    echo "--> State of array before loop:\n";
    var_dump($a);

    echo "--> Do loop:\n";
    foreach ($a as $k=>$v) {
        echo "     iteration $counter:  \$k=$k; \$v=$v\n";
        $transform(inout $a, $k, $counter);
        $counter++;
        if ($counter>MAX_LOOPS) {
            echo "  ** Stuck in a loop! **\n";
            break;
        }
    }

    echo "--> State of array after loop:\n";
    var_dump($a);
}

<<__EntryPoint>> function main(): void {
echo "\nPopping elements off end of an unreferenced array";
$transform = (inout $a, $k, $counter) ==> array_pop(inout $a);
withRefValue(1, $transform);
withRefValue(2, $transform);
withRefValue(3, $transform);
withRefValue(4, $transform);

echo "\n\n\nShift elements off start of an unreferenced array";
$transform = (inout $a, $k, $counter) ==> array_shift(inout $a);
withRefValue(1, $transform);
withRefValue(2, $transform);
withRefValue(3, $transform);
withRefValue(4, $transform);

echo "\n\n\nRemove current element of an unreferenced array";
$transform = (inout $a, $k, $counter) ==> { unset($a[$k]); };
withRefValue(1, $transform);
withRefValue(2, $transform);
withRefValue(3, $transform);
withRefValue(4, $transform);

echo "\n\n\nAdding elements to the end of an unreferenced array";
$transform = (inout $a, $k, $counter) ==> array_push(inout $a, "new.$counter");
withRefValue(1, $transform);
withRefValue(2, $transform);
withRefValue(3, $transform);
withRefValue(4, $transform);

echo "\n\n\nAdding elements to the start of an unreferenced array";
$transform = (inout $a, $k, $counter) ==> array_unshift(inout $a, "new.$counter");
withRefValue(1, $transform);
withRefValue(2, $transform);
withRefValue(3, $transform);
withRefValue(4, $transform);
}
