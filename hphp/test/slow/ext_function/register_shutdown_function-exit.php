<?hh


<<__EntryPoint>>
function main_register_shutdown_function_exit() :mixed{
register_shutdown_function(function () {
    var_dump('first');
});

register_shutdown_function(function () {
    var_dump('second, start');
    register_shutdown_function(function () {
        var_dump('third, start');
        exit;
        var_dump('third, after exit - FAIL');
    });
    var_dump('second, end');
});

register_shutdown_function(function () {
    var_dump('fourth');
});
}
