<?php 

// Create a resource 
$image = tmpfile(); 

// try to draw a white ellipse 
imageellipse($image, 200, 150, 300, 200, 16777215); 
?> 