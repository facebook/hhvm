#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
	char *hash1, *hash2;
	char *host, *repl_set_name, *db, *username, *auth_hash;
	int port, pid;

	mongo_server_def def1 = { "whisky", 13000, NULL, NULL, NULL, NULL };
	mongo_server_def def2 = { "whisky", 13000, "setA", "phpunit", "derick", "not!" };

	hash1 = mongo_server_create_hash(&def1);
	mongo_server_split_hash(hash1, &host, &port, &repl_set_name, &db, &username, &auth_hash, &pid);
	printf("HASH: %s; host: %s, port: %d, replset: %s, db: %s, username: %s, auth_hash: %s, pid: %d\n",
		hash1, host, port, repl_set_name, db, username, auth_hash, pid);
	free(host);
	free(hash1);

	hash2 = mongo_server_create_hash(&def2);

	mongo_server_split_hash(hash2, &host, &port, &repl_set_name, &db, &username, &auth_hash, &pid);
	printf("HASH: %s; host: %s, port: %d, replset: %s, db: %s, username: %s, auth_hash: %s, pid: %d\n",
		hash2, host, port, repl_set_name, db, username, auth_hash, pid);
	free(host); free(repl_set_name); free(db); free(username); free(auth_hash);

	host = db = username = auth_hash = NULL; port = pid = 0;
	mongo_server_split_hash(hash2, &host, &port, &repl_set_name, NULL, &username, &auth_hash, &pid);
	printf("HASH: %s; host: %s, port: %d, replset: %s, db: %s, username: %s, auth_hash: %s, pid: %d\n",
		hash2, host, port, repl_set_name, db, username, auth_hash, pid);
	free(host); free(repl_set_name); free(username); free(auth_hash);

	host = db = username = auth_hash = NULL; port = pid = 0;
	mongo_server_split_hash(hash2, &host, &port, &repl_set_name, &db, NULL, &auth_hash, &pid);
	printf("HASH: %s; host: %s, port: %d, replset: %s, db: %s, username: %s, auth_hash: %s, pid: %d\n",
		hash2, host, port, repl_set_name, db, username, auth_hash, pid);
	free(host); free(repl_set_name); free(db); free(auth_hash);

	host = db = username = auth_hash = NULL; port = pid = 0;
	mongo_server_split_hash(hash2, &host, &port, &repl_set_name, &db, NULL, NULL, &pid);
	printf("HASH: %s; host: %s, port: %d, replset: %s, db: %s, username: %s, auth_hash: %s, pid: %d\n",
		hash2, host, port, repl_set_name, db, username, auth_hash, pid);
	free(host); free(repl_set_name); free(db);

	host = db = username = auth_hash = NULL; port = pid = 0;
	mongo_server_split_hash(hash2, &host, &port, &repl_set_name, NULL, &username, NULL, &pid);
	printf("HASH: %s; host: %s, port: %d, replset: %s, db: %s, username: %s, auth_hash: %s, pid: %d\n",
		hash2, host, port, repl_set_name, db, username, auth_hash, pid);
	free(host); free(repl_set_name); free(username);

	free(hash2);

	return 0;
}
