<?hh


<<__EntryPoint>>
function main_pcntl_exec() :mixed{
pcntl_exec("/bin/sh",
             varray[__DIR__."/test_pcntl_exec.sh"],
             darray["name" => "value"]);
}
