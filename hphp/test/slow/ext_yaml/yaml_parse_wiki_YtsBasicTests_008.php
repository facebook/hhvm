<?php
  var_dump(yaml_parse('foo: whatever
bar:
 -
  fruit: apple
  name: steve
  sport: baseball
 - more
 -
  python: rocks
  perl: papers
  ruby: scissorses
'));
?>
