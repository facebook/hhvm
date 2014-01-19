<?hh

function block() { return RescheduleWaitHandle::create(1,1); };

async function f1() { return; }
async function f2() {  }
async function f3() { await f1(); }
async function f4() { return null; }
var_dump(f1()->join());
var_dump(f2()->join());
var_dump(f3()->join());
var_dump(f4()->join());

async function b1() { await block(); return; }
async function b2() { await block(); }
async function b3() { await block(); await f1(); }
async function b4() { await block(); return null; }
var_dump(b1()->join());
var_dump(b2()->join());
var_dump(b3()->join());
var_dump(b4()->join());
