<?php

mkdir("test");
mkdir("test/dir1");
touch("test/dir1/file1.txt");
mkdir("test/dir2");
touch("test/dir2/file2.txt");
symlink("../dir1", "test/dir2/dir1");

// Don't follow
$iterator = new RecursiveDirectoryIterator('test/dir2', 
              FilesystemIterator::SKIP_DOTS);
$iterator = new RecursiveIteratorIterator($iterator, RecursiveIteratorIterator::SELF_FIRST);

echo "DONT FOLLOW SYMLINKS", PHP_EOL;

foreach ($iterator as $path => $dir) {
    echo $path, PHP_EOL;
}

// Follow
$iterator = new RecursiveDirectoryIterator(
              'test/dir2',
              FilesystemIterator::SKIP_DOTS | 
              FilesystemIterator::FOLLOW_SYMLINKS
            );
$iterator = new RecursiveIteratorIterator($iterator, RecursiveIteratorIterator::SELF_FIRST);

echo "FOLLOW SYMLINKS", PHP_EOL;

foreach ($iterator as $path => $dir) {
    echo $path, PHP_EOL;
}


// Clean up
unlink("test/dir2/dir1");
unlink("test/dir2/file2.txt");
unlink("test/dir1/file1.txt");
rmdir("test/dir2");
rmdir("test/dir1");
rmdir("test");

