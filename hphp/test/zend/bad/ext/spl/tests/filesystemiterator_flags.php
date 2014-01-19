<?php

$it = new FileSystemIterator(".");
printflags($it);

$it->setFlags(FileSystemIterator::CURRENT_AS_SELF |
		FileSystemIterator::KEY_AS_FILENAME |
		FileSystemIterator::SKIP_DOTS | 
		FileSystemIterator::UNIX_PATHS);
printflags($it);

$it->setFlags(-1);
printflags($it);

function printflags($it) {
	printf("%08X\n", $it->getFlags());
	printf("%08X\n", ($it->getFlags() & FileSystemIterator::CURRENT_MODE_MASK));
	printf("%08X\n", ($it->getFlags() & FileSystemIterator::KEY_MODE_MASK));
	printf("%08X\n", ($it->getFlags() & FileSystemIterator::OTHER_MODE_MASK));
}

?>