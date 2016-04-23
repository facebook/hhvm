<?php
/**
 * Regular Phar
 */
echo ".phar:\n";
var_dump((new Phar(__DIR__."/basic.phar"))->isCompressed());

echo "\n.phar:\n";
var_dump((new Phar(__DIR__."/basic.phar"))->isCompressed());

echo "\n.phar.gz:\n";
var_dump((new Phar(__DIR__."/basic.phar.gz"))->isCompressed());

echo "\n.phar.bz2:\n";
var_dump((new Phar(__DIR__."/basic.phar.bz2"))->isCompressed());

/**
 * Tar-based Phar
 */
echo "\n.phar.tar:\n";
var_dump((new Phar(__DIR__."/basic.phar.tar"))->isCompressed());

echo "\n.phar.tar.gz:\n";
var_dump((new Phar(__DIR__."/basic.phar.tar.gz"))->isCompressed());

echo "\n.phar.tar.bz2:\n";
var_dump((new Phar(__DIR__."/basic.phar.tar.bz2"))->isCompressed());

/**
 * Zip-based Phar
 */
echo "\n.phar.zip:\n";
var_dump((new Phar(__DIR__."/basic.phar.zip"))->isCompressed());

/**
 * Tar with extension of regular Phar
 */
echo "\n.phar (which is .phar.tar):\n";
var_dump((new Phar(__DIR__."/basic-tar.phar"))->isCompressed());

/**
 * Tar GZ with extension of regular Phar
 */
echo "\n.phar (which is .phar.tar.gz):\n";
var_dump((new Phar(__DIR__."/basic-tar-gz.phar"))->isCompressed());

/**
 * Tar BZ2 with extension of regular Phar
 */
echo "\n.phar (which is .phar.tar.bz2):\n";
var_dump((new Phar(__DIR__."/basic-tar-bz2.phar"))->isCompressed());

/**
 * Zip with extension of regular Phar
 */
echo "\n.phar (which is .phar.zip):\n";
var_dump((new Phar(__DIR__."/basic-zip.phar"))->isCompressed());
