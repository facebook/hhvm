<?hh

function plop() :mixed{
    $ts = time();
    while(true) {
        if ((time()-$ts) > 2) {
            echo "Failed!";
            break;
        }
    }
}

<<__EntryPoint>> function main(): void {
set_time_limit(1);
register_shutdown_function(plop<>);

plop();
echo "===DONE===\n";
}
