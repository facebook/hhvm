<?hh
setcookie($_GET['cookie_name'],
          $_GET['cookie_value'],
          0,
          (string)($_GET['cookie_path'] ?? ""),
          (string)($_GET['cookie_domain'] ?? ""));
var_dump($_GET['cookie_name']);
