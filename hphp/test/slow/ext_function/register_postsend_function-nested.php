<?hh


<<__EntryPoint>>
function main_register_postsend_function_nested() :mixed{
var_dump('before reg');

register_postsend_function(function() {
  var_dump('first, start');
  register_postsend_function(function() {
    var_dump('second, start');
    register_postsend_function(function() {
      var_dump('third, start');
      register_postsend_function(function() {
        var_dump('fourth, start');
      });
      var_dump('third, end');
    });
    var_dump('second, end');
  });
  var_dump('first, end');
});

var_dump('after reg');
}
