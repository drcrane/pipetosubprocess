#include <stdio.h>
#include <malloc.h>
#include <stddef.h>
#include <stdlib.h>

#include "emailer.h"

int main(int argc, char* argv[]) {
	char * email;
	int res;
	email = emailer_createreminder(argv[1], argv[2]);
	if (email == NULL) {
		fprintf(stderr, "Failed to create email content\n");
		return 1;
	}
	res = emailer_pipetomsmtp(email);
	free(email);
	if (res == 0) {
		fprintf(stdout, "Success!\n");
	} else {
		fprintf(stderr, "Nope, Failure.\n");
		return 1;
	}
	return 0;
}

