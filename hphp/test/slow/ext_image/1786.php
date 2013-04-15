<?php

 // Create an image instance$im = imagecreatefromgif('test/images/php.gif');// Enable interlancingimageinterlace($im, true);// Save the interfaced imageimagegif($im, './php_interlaced.gif');imagedestroy($im);