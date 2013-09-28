<?php
$m_envelope["To"] = "mail@example.com";
$m_part1["type"] = TYPEMULTIPART;
$m_part1["subtype"] = "mixed";
$m_part2["type"] = TYPETEXT;
$m_part2["subtype"] = "plain";
$m_part2["description"] = "text_message";

$m_part2["charset"] = "ISO-8859-2";

$m_part2["contents.data"] = "hello";
$m_body[1] = $m_part1;
$m_body[2] = $m_part2;
echo imap_mail_compose($m_envelope, $m_body);
?>