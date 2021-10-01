<?hh

function f((function ()[_]: void) $f)[ctx $f]: void {}

function f_option(?(function ()[_]: void) $f)[ctx $f]: void {}

function f_like(~(function ()[_]: void) $f)[ctx $f]: void {}

function f_like_option(~?(function ()[_]: void) $f)[ctx $f]: void {}
