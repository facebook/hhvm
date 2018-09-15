<?hh

class ctor {
  async function __construct() {}
}

<<__EntryPoint>>
function main_async_constructor() {
new ctor;
echo "done";
}
