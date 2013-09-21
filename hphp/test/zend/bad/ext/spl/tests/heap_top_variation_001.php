<?php
$h = new SplMinHeap();
$h->insert(5);
// top doesn't take any args, lets see what happens if we give it one
$h->top('bogus');
?>