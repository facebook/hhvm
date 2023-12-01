<?hh

function provideDarrayOfStringToInt(): darray<string, int> {
  return dict["tingley" => 0, "meijer" => 1, "dreeves" => 2];
}

function provideDarrayOfMixedToArraykey(): darray<mixed, arraykey> {
  return provideDarrayOfStringToInt();
}
