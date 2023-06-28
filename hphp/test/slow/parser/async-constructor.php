<?hh

class ctor {
  async function __construct()[] {}
}

<<__EntryPoint>>
function main_async_constructor() :mixed{
new ctor;
echo "done";
}
