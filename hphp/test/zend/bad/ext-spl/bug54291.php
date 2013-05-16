<?php
$dir = new DirectoryIterator("\x00/abc");
$dir->isFile();