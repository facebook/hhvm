<?php


<<__EntryPoint>>
function main_silenced_exit() {
print "Before silenced error\n";
@blah();
print "After silenced error, this should not show!\n";
}
