<?php
/**
 * Regular Phar
 */
echo ".phar:\n";
var_dump((new Phar(__DIR__."/basic.phar"))->getSignature());

echo "\n.phar:\n";
var_dump((new Phar(__DIR__."/basic.phar"))->getSignature());

echo "\n.phar.gz:\n";
var_dump((new Phar(__DIR__."/basic.phar.gz"))->getSignature());

echo "\n.phar.bz2:\n";
var_dump((new Phar(__DIR__."/basic.phar.bz2"))->getSignature());

/**
 * Tar-based Phar
 */
echo "\n.phar.tar:\n";
var_dump((new Phar(__DIR__."/basic.phar.tar"))->getSignature());

echo "\n.phar.tar.gz:\n";
var_dump((new Phar(__DIR__."/basic.phar.tar.gz"))->getSignature());

echo "\n.phar.tar.bz2:\n";
var_dump((new Phar(__DIR__."/basic.phar.tar.bz2"))->getSignature());

/**
 * Zip-based Phar
 */
echo "\n.phar.zip:\n";
var_dump((new Phar(__DIR__."/basic.phar.zip"))->getSignature());

/**
 * Tar with extension of regular Phar
 */
echo "\n.phar (which is .phar.tar):\n";
var_dump((new Phar(__DIR__."/basic-tar.phar"))->getSignature());

/**
 * Tar GZ with extension of regular Phar
 */
echo "\n.phar (which is .phar.tar.gz):\n";
var_dump((new Phar(__DIR__."/basic-tar-gz.phar"))->getSignature());

/**
 * Tar BZ2 with extension of regular Phar
 */
echo "\n.phar (which is .phar.tar.bz2):\n";
var_dump((new Phar(__DIR__."/basic-tar-bz2.phar"))->getSignature());

/**
 * Zip with extension of regular Phar
 */
echo "\n.phar (which is .phar.zip):\n";
var_dump((new Phar(__DIR__."/basic-zip.phar"))->getSignature());
