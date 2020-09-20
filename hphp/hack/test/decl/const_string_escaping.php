<?hh

class C {
  const string SINGLE = 'whomst\'d\'ve';
  const string DOUBLE = "\n\t";
  const string SINGLE_ENDS_IN_SINGLE = 'a\'';
  const string DOUBLE_ENDS_IN_DOUBLE = "a\"";
  const string SINGLE_ESCAPED_BACKSLASH = '\\';
  const string DOUBLE_ESCAPED_BACKSLASH = "\\";
  const string SINGLE_ESCAPED_DOLLAR = '\$';
  const string DOUBLE_ESCAPED_DOLLAR = "\$";
  const string HEREDOC = <<<EOT
    \n\t
EOT;
  const string NOWDOC = <<<'EOT'
    no escape sequences are unescaped: \'\n\t
EOT;
}
