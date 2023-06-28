<?hh


// chunk size of 2
<<__EntryPoint>>
function main_ob_start_chunk() :mixed{
ob_start(function ($s) { return 'ob: ' . $s; }, 2);

for ($i = 1; $i <= 5; $i++) {
        echo $i . "\n";
}

ob_end_flush();

// No chunk
ob_start(function ($s) { return 'ob: ' . $s; });

for ($i = 6; $i <= 10; $i++) {
        echo $i . "\n";
}

ob_end_flush();


// 5 chunk, no newline
ob_start(function ($s) { return 'ob: ' . $s; }, 5);

for ($i = 111; $i <= 115; $i++) {
        echo $i;
}

ob_end_flush();


// 13 chunk, random
ob_start(function ($s) { return 'ob: ' . $s; }, 13);
echo "I am going ";
echo "to see if this ";
echo "chunking works well\n";
echo "I sure hope it does; ";
echo "otherwise we debug!\n";

ob_end_flush();
}
