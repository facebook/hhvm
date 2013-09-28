<?php
  var_dump(yaml_parse('
# Explicitly typed ordered map (dictionary).
Bestiary: !!omap
  - aardvark: African pig-like ant eater. Ugly.
  - anteater: South-American ant eater. Two species.
  - anaconda: South-American constrictor snake. Scaly.
  # Etc.
# Flow style
Numbers: !!omap [ one: 1, two: 2, three : 3 ]
'));
?>
