#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#define SIZE BUFSIZ
#define NAME "tldbf"
#define DELIM " \t\r\n"
#define TLDFILE "tlds-alpha-by-domain.txt"

/* https://www.icann.org/news/announcement-2-2014-08-01-en */
#define ICAN_SPECIAL_IP 892665983 /* 127.0.53.53 */

int verbose = 0;

char *pathlist[] = {
	".",
	"/usr/share/"NAME,
	NULL
};

void strtolower(char *buf, size_t size)
{
	unsigned int i;

	for (i = 0; buf[i] && i < size; i++)
		buf[i] = tolower(buf[i]);
}

void normalize (char *buf)
{
	char *ptr;
	unsigned int len;

	len = strlen(buf);
	ptr = buf + len - 1;
	while (ptr > buf) {
	
		if (!strchr(DELIM, *ptr))
			break;

		*ptr-- = 0;
	}

	strtolower(buf, len);
}

char *inet_ntoa (struct in_addr addr)
{
	unsigned int ip;
	static char ipstr[32];

	ip = addr.s_addr;

	snprintf(ipstr, sizeof(ipstr), "%u.%u.%u.%u",
		((ip >>  0) & 0xff),
		((ip >>  8) & 0xff),
		((ip >> 16) & 0xff),
		((ip >> 24) & 0xff));

	return ipstr;
}

void print_domain (char *domain)
{
	unsigned int i;
	struct hostent *host;

	if (verbose)
		printf("-- Resolving domain %s\n", domain);

	host = gethostbyname(domain);
	if (!host)
		return;

	if (host->h_addr_list[0] && !host->h_addr_list[1] && (*(struct in_addr *)host->h_addr_list[0]).s_addr == ICAN_SPECIAL_IP)
		return;

	if (verbose)
		printf("\nFound valid domain ");

	printf("%s (", domain);

	for (i = 0; host->h_addr_list[i]; i++) {

		printf("%s", inet_ntoa(*(struct in_addr *)host->h_addr_list[i]));

		if (host->h_addr_list[i + 1])
			printf(", ");
	}

	printf(")\n");
}

void bf_tlds (char *file, char *name)
{
	FILE *fp;
	char tld[SIZE];
	char domain[SIZE];

	fp = fopen(file, "r");
	if (!fp) {
		perror("fopen failed");
		return;
	}

	while (fgets(tld, SIZE, fp)) {

		normalize(tld);

		if (strchr(" \t\r\n#", tld[0]))
			continue;

		strtolower(tld, SIZE);
		snprintf(domain, SIZE, "%s.%s", name, tld);
		print_domain(domain);
	}

	fclose(fp);
}

int check_valid_name (char *name)
{
	unsigned int i, len;

	len = strlen(name);
	for (i = 0; i < len; i++) {

		if (name[i] >= 'a' && name[i] <= 'z')
			continue;

		if (name[i] >= 'A' && name[i] <= 'Z')
			continue;

		if (name[i] >= '0' && name[i] <= '9')
			continue;

		if (name[i] == '.' || name[i] == '-')
			continue;

		return 0;
	}

	return 1;
}

char *check_file_exists (char *path)
{
	int err;
	struct stat info;

	memset(&info, 0, sizeof(info));
	err = stat(path, &info);
	return (err ? NULL : path);
}

char *check_tldfile_path (char *dir, char *name)
{
	int len;
	static char path[BUFSIZ];

	len = snprintf(path, sizeof(path), "%s/%s", dir, name);
	if (len < 0 || (unsigned)len >= sizeof(path)) {
		perror("snprintf failed");
		return NULL;
	}

	return check_file_exists(path);
}

char *find_tldfile_path ()
{
	unsigned int i;
	char *path;

	for (i = 0; pathlist[i]; i++) {

		path = check_tldfile_path(pathlist[i], TLDFILE);
		if (path)
			return path;
	}

	return NULL;
}

int main (int argc, char **argv)
{
	char *name, *ptr, *tldfile;

	if (argc < 2) {
		printf("Usage: %s <name>\n", argv[0]);
		return EXIT_FAILURE;
	}

	name = argv[1];
	ptr = strrchr(name, '.');
	if (ptr) {

		*ptr = 0;
		ptr = strrchr(name, '.');
		if (ptr)
			name = ++ptr;
	}

	if (!check_valid_name(name)) {
		printf("Invalid name `%s'\n", name);
		return EXIT_FAILURE;
	}

	tldfile = find_tldfile_path();
	if (!tldfile) {
		printf("Could not find TLD file: %s\n", TLDFILE);
		return EXIT_FAILURE;
	}

	bf_tlds(tldfile, name);

	return EXIT_SUCCESS;
}

