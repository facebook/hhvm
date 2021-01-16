<?hh

function poly<Texplicit as D>(
  C $change, // Type hint replaced with T$change
  ?C $nchange, // Type hint replaced with ?T$nchange
  Texplicit $remain,
  ?Texplicit $nremain,
)[$change::Co1, $nchange::Co2, $remain::Co3, $nremain::Co4]: void {}
