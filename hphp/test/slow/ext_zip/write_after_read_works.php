<?php
// Create test ZIP file
$zip_setup = new ZipArchive();
$zip_setup->open('hello.zip', ZipArchive::OVERWRITE | ZipArchive::CREATE);
$zip_setup->addFromString('hello.txt', 'Old value here');
$zip_setup->addFromString('hello2.txt', 'Old value here');
$zip_setup->close();

// Load the file fresh to modify it
$zip = new ZipArchive();
$zip->open('hello.zip');

// It works if you comment out this line
echo 'Old value: ', $zip->getFromName('hello.txt'), "\n";
$new = 'It worked at '.time();
$zip->addFromString('hello.txt', $new);
$zip->addFromString('hello2.txt', $new);
echo 'New value: ', $new, "\n";
$zip->close();

// Reload the ZIP to show that it hasn't actually been modified
$zip = new ZipArchive();
$zip->open('hello.zip');
echo 'Reloaded value hello.txt: ', $zip->getFromName('hello.txt'), "\n";
echo 'Reloaded value hello2.txt: ', $zip->getFromName('hello2.txt'), "\n";

unlink('hello.zip');
