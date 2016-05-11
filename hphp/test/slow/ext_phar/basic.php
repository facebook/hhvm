<?php
/**
 * Regular Phar
 */
echo ".phar:\n";
include __DIR__."/basic.phar";

echo "\n.phar with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic.phar";

// TODO: not working somewhere deep inside HHVM; if you know how to fix - do it
//echo "\n.phar.gz:\n";
//include __DIR__."/basic.phar.gz";

echo "\n.phar.gz with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic.phar.gz";

// TODO: not working somewhere deep inside HHVM; if you know how to fix - do it
//echo "\n.phar.bz2:\n";
//include __DIR__."/basic.phar.bz2";

echo "\n.phar.bz2 with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic.phar.bz2";

/**
 * Tar-based Phar
 */
// TODO: not working somewhere deep inside HHVM; if you know how to fix - do it
//echo "\n.phar.tar:\n";
//include __DIR__."/basic.phar.tar";

echo "\n.phar.tar with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic.phar.tar";

// TODO: not working somewhere deep inside HHVM; if you know how to fix - do it
//echo "\n.phar.tar.gz:\n";
//include __DIR__."/basic.phar.tar.gz";

echo "\n.phar.tar.gz with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic.phar.tar.gz";

// TODO: not working somewhere deep inside HHVM; if you know how to fix - do it
//echo "\n.phar.tar.bz2:\n";
//include __DIR__."/basic.phar.tar.bz2";

echo "\n.phar.tar.bz2 with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic.phar.tar.bz2";

/**
 * Zip-based Phar
 */
// TODO: not working somewhere deep inside HHVM; if you know how to fix - do it
//echo "\n.phar.zip:\n";
//include __DIR__."/basic.phar.zip";

echo "\n.phar.zip with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic.phar.zip";

/**
 * Tar with extension of regular Phar
 */
// TODO: not working somewhere deep inside HHVM; if you know how to fix - do it
//echo ".phar (which is .phar.tar):\n";
//include __DIR__."/basic-tar.phar";

echo "\n.phar (which is .phar.tar) with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic-tar.phar";

/**
 * Tar GZ with extension of regular Phar
 */
// TODO: not working somewhere deep inside HHVM; if you know how to fix - do it
//echo ".phar (which is .phar.tar.gz):\n";
//include __DIR__."/basic-tar-gz.phar";

echo "\n.phar (which is .phar.tar.gz) with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic-tar-gz.phar";

/**
 * Tar BZ2 with extension of regular Phar
 */
// TODO: not working somewhere deep inside HHVM; if you know how to fix - do it
//echo ".phar (which is .phar.tar.bz2):\n";
//include __DIR__."/basic-tar-bz2.phar";

echo "\n.phar (which is .phar.tar.bz2) with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic-tar-bz2.phar";

/**
 * Zip with extension of regular Phar
 */
// TODO: not working somewhere deep inside HHVM; if you know how to fix - do it
//echo ".phar (which is .phar.zip):\n";
//include __DIR__."/basic-zip.phar";

echo "\n.phar (which is .phar.zip) with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic-zip.phar";
