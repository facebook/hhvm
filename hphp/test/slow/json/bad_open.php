<?php
$json = '{"foo": { "package": { "bar": "b{az" }}}}';
var_dump(json_decode($json));
