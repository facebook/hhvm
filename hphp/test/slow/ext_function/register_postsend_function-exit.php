<?hh


<<__EntryPoint>>
function main_register_postsend_function_exit() :mixed{
register_postsend_function(function () {
    var_dump('first');
});

register_postsend_function(function () {
    var_dump('second, start');
    register_postsend_function(function () {
        var_dump('third, start');
        exit;
        var_dump('third, after exit - FAIL');
    });
    var_dump('second, end');
});

register_postsend_function(function () {
    var_dump('fourth');
});
}
