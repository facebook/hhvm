<?hh // strict

function f(): void {
  $good0 = re"/((Banana))/ux";
  $good1 = re"(()(<>)((()(<>))))x";
  $good2 = re"<<>>x";
  $good3 = re"(())x";
  $good4 = re"((()()))";
  $good5 = re"<\>>x";
  $good6 = re"[<]";
  $bad0  = re"[][]x";
  $bad1  = re"[\()]x";
  $bad2  = re"(\)";
  $bad3  = re"(()";
  $bad4  = re"<<<>>";
  $bad5  = re"[[]]";
}
