<?php

$regex = <<<'REGEX'
/ 
    _ (a) (*MARK:A_MARK) _
  | _ (b) _
  | _ (c) (*MARK:C_MARK) _
  | _ (d) _
/x
REGEX;

var_dump(preg_match($regex, '_c_', $matches));
var_dump($matches);

var_dump(preg_match_all($regex, '_a__b__c__d_', $matches, PREG_PATTERN_ORDER));
var_dump($matches);

var_dump(preg_match_all($regex, '_a__b__c__d_', $matches, PREG_SET_ORDER));
var_dump($matches);

var_dump(preg_replace_callback($regex, function($matches) {
    var_dump($matches);
    return $matches[0];
}, '_a__b__c__d_'));

?>
