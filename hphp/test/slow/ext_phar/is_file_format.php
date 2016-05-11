<?php
/**
 * Regular Phar
 */
echo ".phar:\n";
$p = new Phar(__DIR__."/basic.phar");
var_dump('is phar', $p->isFileFormat(Phar::PHAR));
var_dump('is tar', $p->isFileFormat(Phar::TAR));
var_dump('is zip', $p->isFileFormat(Phar::ZIP));

echo "\n.phar:\n";
$p = new Phar(__DIR__."/basic.phar");
var_dump('is phar', $p->isFileFormat(Phar::PHAR));
var_dump('is tar', $p->isFileFormat(Phar::TAR));
var_dump('is zip', $p->isFileFormat(Phar::ZIP));

echo "\n.phar.gz:\n";
$p = new Phar(__DIR__."/basic.phar.gz");
var_dump('is phar', $p->isFileFormat(Phar::PHAR));
var_dump('is tar', $p->isFileFormat(Phar::TAR));
var_dump('is zip', $p->isFileFormat(Phar::ZIP));

echo "\n.phar.bz2:\n";
$p = new Phar(__DIR__."/basic.phar.bz2");
var_dump('is phar', $p->isFileFormat(Phar::PHAR));
var_dump('is tar', $p->isFileFormat(Phar::TAR));
var_dump('is zip', $p->isFileFormat(Phar::ZIP));

/**
 * Tar-based Phar
 */
echo "\n.phar.tar:\n";
$p = new Phar(__DIR__."/basic.phar.tar");
var_dump('is phar', $p->isFileFormat(Phar::PHAR));
var_dump('is tar', $p->isFileFormat(Phar::TAR));
var_dump('is zip', $p->isFileFormat(Phar::ZIP));

echo "\n.phar.tar.gz:\n";
$p = new Phar(__DIR__."/basic.phar.tar.gz");
var_dump('is phar', $p->isFileFormat(Phar::PHAR));
var_dump('is tar', $p->isFileFormat(Phar::TAR));
var_dump('is zip', $p->isFileFormat(Phar::ZIP));

echo "\n.phar.tar.bz2:\n";
$p = new Phar(__DIR__."/basic.phar.tar.bz2");
var_dump('is phar', $p->isFileFormat(Phar::PHAR));
var_dump('is tar', $p->isFileFormat(Phar::TAR));
var_dump('is zip', $p->isFileFormat(Phar::ZIP));

/**
 * Zip-based Phar
 */
echo "\n.phar.zip:\n";
$p = new Phar(__DIR__."/basic.phar.zip");
var_dump('is phar', $p->isFileFormat(Phar::PHAR));
var_dump('is tar', $p->isFileFormat(Phar::TAR));
var_dump('is zip', $p->isFileFormat(Phar::ZIP));

/**
 * Tar with extension of regular Phar
 */
echo "\n.phar (which is .phar.tar):\n";
$p = new Phar(__DIR__."/basic-tar.phar");
var_dump('is phar', $p->isFileFormat(Phar::PHAR));
var_dump('is tar', $p->isFileFormat(Phar::TAR));
var_dump('is zip', $p->isFileFormat(Phar::ZIP));

/**
 * Tar GZ with extension of regular Phar
 */
echo "\n.phar (which is .phar.tar.gz):\n";
$p = new Phar(__DIR__."/basic-tar-gz.phar");
var_dump('is phar', $p->isFileFormat(Phar::PHAR));
var_dump('is tar', $p->isFileFormat(Phar::TAR));
var_dump('is zip', $p->isFileFormat(Phar::ZIP));

/**
 * Tar BZ2 with extension of regular Phar
 */
echo "\n.phar (which is .phar.tar.bz2):\n";
$p = new Phar(__DIR__."/basic-tar-bz2.phar");
var_dump('is phar', $p->isFileFormat(Phar::PHAR));
var_dump('is tar', $p->isFileFormat(Phar::TAR));
var_dump('is zip', $p->isFileFormat(Phar::ZIP));

/**
 * Zip with extension of regular Phar
 */
echo "\n.phar (which is .phar.zip):\n";
$p = new Phar(__DIR__."/basic-zip.phar");
var_dump('is phar', $p->isFileFormat(Phar::PHAR));
var_dump('is tar', $p->isFileFormat(Phar::TAR));
var_dump('is zip', $p->isFileFormat(Phar::ZIP));
