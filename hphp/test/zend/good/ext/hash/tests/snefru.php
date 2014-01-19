<?php
echo hash('snefru', ''), "\n";
echo hash('snefru', 'The quick brown fox jumps over the lazy dog'), "\n";
echo hash('snefru', 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'), "\n";
echo hash('snefru', 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'), "\n";
echo hash('snefru', 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'), "\n";
?>