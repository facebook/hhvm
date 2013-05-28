<?php

function foo($where_clause){
  $sql =    'SELECT p.property_name, p.property_value, p.property_id, '.    'p.property_data_type, p.property_type, '.    't.tier_id, t.tier_parent_id, t.tier_version, t.tier_type, '.    't.tier_state, t.tier_name '.    'FROM tier t LEFT OUTER JOIN property p ON '.    'p.parent_id = t.tier_id AND '.    'p.property_type = "tier" '.    $where_clause;
  echo $sql . "\n";
}
foo("where 1 = 1");
