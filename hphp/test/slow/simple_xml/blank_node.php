<?php

$config = simplexml_load_string(<<<EOF
<config>
  <global>
    <index>
      <indexer>

      </indexer>
    </index>
  </global>
</config>
EOF
);

var_dump($config->global->index);
