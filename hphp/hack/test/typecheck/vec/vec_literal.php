<?hh // strict

function empty_vec_literal(): vec<string> {
  return vec[];
}

function filled_vec_literal(): vec<string> {
  return vec['string'];
}

function filled_vec_literal_multi(): vec<int> {
  return vec[
    42,
    11, // trailing commas!
  ];
}
