<?hh


<<__EntryPoint>>
function main_pcntl_exec() :mixed{
pcntl_exec("/bin/sh",
             vec[__DIR__."/test_pcntl_exec.sh"],
             dict["name" => "value"]);
}
