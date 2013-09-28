<?php
  var_dump(yaml_parse('
---
time: !!str 20:03:20
player: Sammy Sosa
action: strike (miss)
...

%YAML 1.1
---
time: "20:03:47"
player: Sammy Sosa
action: grand slam
...
', -1));
?>
