<?php


<<__EntryPoint>>
function main_backquote() {
echo `echo $(echo hello)`;
echo `echo \`echo hello\``;
}
