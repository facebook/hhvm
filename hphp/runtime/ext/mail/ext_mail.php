<?hh
/* Sends an email.
 * @param string $to
 * @param string $subject
 * @param string $message - Message to be sent.  Each line should be separated
 * with a LF (\n). Lines should not be larger than 70 characters. Caution
 * (Windows only) When PHP is talking to a SMTP server directly, if a full
 * stop is found on the start of a line, it is removed. To counter-act this,
 * replace these occurrences with a double dot.
 * @param string $additional_headers
 * @param string $additional_parameters - The additional_parameters parameter
 * can be used to pass additional flags as command line options to the program
 * configured to be used when sending mail, as defined by the sendmail_path
 * configuration setting. For example, this can be used to set the envelope
 * sender address when using sendmail with the -f sendmail option.  The user
 * that the webserver runs as should be added as a trusted user to the
 * sendmail configuration to prevent a 'X-Warning' header from being added to
 * the message when the envelope sender (-f) is set using this method. For
 * sendmail users, this file is /etc/mail/trusted-users.
 * @return bool - Returns TRUE if the mail was successfully accepted for
 * delivery, FALSE otherwise.  It is important to note that just because the
 * mail was accepted for delivery, it does NOT mean the mail will actually
 * reach the intended destination.
 */
<<__Native>>
function mail(string $to,
              string $subject,
              string $message,
              string $additional_headers = "",
              string $additional_parameters = ""): bool;

/* @param string $addr - The email address that's being hashed.
 * @return int - The hash value of addr.
 */
<<__Native>>
function ezmlm_hash(string $addr): int;
