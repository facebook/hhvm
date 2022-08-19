DROP FUNCTION IF EXISTS extract_schema_from_file_name_linux;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' FUNCTION extract_schema_from_file_name_linux (
        path VARCHAR(512)
    )
    RETURNS VARCHAR(64)
    SQL SECURITY INVOKER
    DETERMINISTIC
    NO SQL
BEGIN
    RETURN LEFT(SUBSTRING_INDEX(SUBSTRING_INDEX(path, '/', -2), '/', 1), 64);
END$$

DELIMITER ;
