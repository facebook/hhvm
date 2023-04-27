This test exercises the `Autoload.TrustedDBPath` setting. You can see this setting in the `.hhvmconfig.hdf` file found within this directory.

`Autoload.TrustedDBPath` is used to set a read-only SQLite DB file as a trusted repository of information about this directory. If we're using this setting, we will not crawl the filesystem or use Watchman to watch the filesystem. We will instead trust all information within the given DB.

To update this test, you will also need to update the trusted DB found at the place that `Autoload.TrustedDBPath` points to. You can use the `sqlite3` CLI tool to insert rows into the DB, or you can generate a new DB:

1. Disable the `Autoload.TrustedDBPath` setting within `.hhvmconfig.hdf`
2. Run this test again. It will run as a normal test with native autoloading, and will generate a SQLite3 DB as part of its normal operation.
3. Find the DB that this test generated.
4. Copy this newly-generated DB to the place given in `Autoload.TrustedDBPath`.
5. Reenable the `Autoload.TrustedDBPath` setting.
6. Run the test again.

If you do so, you may find that rerunning this test works but creates two extraneous files, such as `trusted-autoload.php.0.autoloadDB-shm` and `trusted-autoload.php.0.autoloadDB-wal`. SQLite creates these files whenever you open a DB that's in WAL mode. To stop these files from being generated every time you run the test, open the DB with `sqlite3` and run `PRAGMA journal_mode = OFF` to disable journaling. Since the DB is read-only, it doesn't need SQLite's journaling features.
