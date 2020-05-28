<?hh

<<__Deprecated('single'.'single')>>
function singlesingle(): void {}

<<__Deprecated("double"."double")>>
function doubledouble(): void {}

<<__Deprecated('single'."double")>>
function singledouble(): void {}

<<__Deprecated('single'."double".'single')>>
function singledoublesingle(): void {}

<<__Deprecated('single'.<<<EOT
  heredoc
EOT
)>>
function heredoc(): void {}

<<__Deprecated('single'.<<<'EOT'
  nowdoc
EOT
)>>
function nowdoc(): void {}
