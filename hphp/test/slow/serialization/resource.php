<?php

var_dump(serialize(fopen(__DIR__.'/resource.txt', 'r')));
var_dump(serialize(fopen('non_existing.txt', 'r')));
