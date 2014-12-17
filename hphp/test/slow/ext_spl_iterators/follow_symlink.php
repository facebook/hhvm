<?php
$p = '/tmp/recursivedirectoryiterator_followsymlink_test/';

mkdir($p);
mkdir($p . "test");
mkdir($p . "test/dir1");
touch($p . "test/dir1/file1.txt");
mkdir($p . "test/dir2");
touch($p . "test/dir2/file2.txt");
symlink($p . "../dir1", $p . "test/dir2/dir1");

// Don't follow
$iterator = new RecursiveDirectoryIterator($p . 'test/dir2', 
              FilesystemIterator::SKIP_DOTS);
$iterator = new RecursiveIteratorIterator($iterator,
              RecursiveIteratorIterator::SELF_FIRST);

echo "DONT FOLLOW SYMLINKS", PHP_EOL;

foreach ($iterator as $path => $dir) {
    echo $path, PHP_EOL;
}

// Follow
$iterator = new RecursiveDirectoryIterator(
              $p . 'test/dir2',
              FilesystemIterator::SKIP_DOTS | 
              FilesystemIterator::FOLLOW_SYMLINKS
            );
$iterator = new RecursiveIteratorIterator($iterator,
              RecursiveIteratorIterator::SELF_FIRST);

echo "FOLLOW SYMLINKS", PHP_EOL;

foreach ($iterator as $path => $dir) {
    echo $path, PHP_EOL;
}


// Clean up
unlink($p . "test/dir2/dir1");
unlink($p . "test/dir2/file2.txt");
unlink($p . "test/dir1/file1.txt");
rmdir($p . "test/dir2");
rmdir($p . "test/dir1");
rmdir($p . "test");
rmdir($p);

