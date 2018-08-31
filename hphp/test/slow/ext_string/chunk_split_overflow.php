<?php


<<__EntryPoint>>
function main_chunk_split_overflow() {
chunk_split(str_repeat('*', 2000000), 1.0, str_repeat('*', 2000000));
}
