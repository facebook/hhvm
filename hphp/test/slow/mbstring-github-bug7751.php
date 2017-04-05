<?php

@mb_parse_str("preds=status|=|3;id|>|1000&limit=1",$output);
var_dump($output);
