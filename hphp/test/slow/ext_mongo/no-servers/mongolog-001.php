<?php
// Common constants
var_dump(0 === MongoLog::NONE);
var_dump(31 === MongoLog::ALL);

// MongoLog::setLevel() constants
var_dump(1 === MongoLog::WARNING);
var_dump(2 === MongoLog::INFO);
var_dump(4 === MongoLog::FINE);

// MongoLog::setModule() constants
var_dump(1 === MongoLog::RS);
var_dump(1 === MongoLog::POOL); // This constant is mapped to ::RS for BC
var_dump(4 === MongoLog::IO);
var_dump(8 === MongoLog::SERVER);
var_dump(16 === MongoLog::PARSE);
?>
