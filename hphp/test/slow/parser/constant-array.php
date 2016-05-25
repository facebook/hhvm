<?php

//NOTE: RepoAuth mode doesn't properly handle the first constant test,
//      a seperate .expectf-repo is included till that is resovled.
echo great[1];
echo "\n";
define("great","my");
echo great[1];
