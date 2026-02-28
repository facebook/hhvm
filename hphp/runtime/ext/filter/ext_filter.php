<?hh

/**
 * Returns a list of all supported filters
 *
 * @return mixed - Returns an array of names of all supported filters, empty
 *   array if there are no such filters. Indexes of this array are not filter
 *   IDs, they can be obtained with filter_id() from a name instead.
 *
 */
<<__Native>>
function filter_list()[]: mixed;

/**
 * Returns the filter ID belonging to a named filter
 *
 * @param mixed $filtername - Name of a filter to get.
 *
 * @return mixed - ID of a filter on success or FALSE if filter doesn't exist.
 *
 */
<<__Native>>
function filter_id(string $filtername)[]: mixed;

/**
 * Filters a variable with a specified filter
 *
 * @return mixed - Returns the filtered data, or FALSE if the filter fails.
 *
 */
<<__Native>>
function filter_var(mixed $variable,
                    int $filter = FILTER_DEFAULT,
                    mixed $options = dict[])[]: mixed;
