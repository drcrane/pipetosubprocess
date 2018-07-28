#define _POSIX_C_SOURCE 200809

#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "dbg.h"
#include "emailer.h"

const static char* Weekdays[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const static char* Months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

char * emailer_createreminder(char * emailaddress, char * token) {
	char * buf;
	size_t buflen;
	size_t bufpos;
	int wlen;
	time_t t;
	struct tm tms;

	bufpos = 0;
	buflen = (1 * 1024 * 1024); // 1Mb buffer
	buf = malloc(buflen);
	check_mem(buf);
	t = time(NULL);
	gmtime_r(&t, &tms);

	wlen = snprintf(buf+bufpos, buflen-bufpos,
			"From: \"Example Sender\" <examplesender@example.com>\n");
	bufpos += wlen;

	// msmtp will read from this field if the --read-recipients argument is given on the
	// command line... which it will be!
	wlen = snprintf(buf+bufpos, buflen-bufpos,
			"To: %s\n",
			emailaddress);
	bufpos += wlen;

	// RFC2822 date format https://tools.ietf.org/html/rfc2822#page-14
	// Stolen from mutt
	wlen = snprintf(buf+bufpos, buflen-bufpos,
			"Date: %s, %d %s %d %02d:%02d:%02d %+03d%02d\n",
			Weekdays[tms.tm_wday], tms.tm_mday, Months[tms.tm_mon],
			tms.tm_year + 1900, tms.tm_hour, tms.tm_min, tms.tm_sec,
			0, 0);
	bufpos += wlen;

	wlen = snprintf(buf+bufpos, buflen-bufpos,
			"Subject: A Subject for this Email\n");
	bufpos += wlen;

	wlen = snprintf(buf+bufpos, buflen-bufpos,
			"Content-Type: text/plain; charset=UTF-8\n");
	bufpos += wlen;

	wlen = snprintf(buf+bufpos, buflen-bufpos,
			"\n");
	bufpos += wlen;

	wlen = snprintf(buf+bufpos, buflen-bufpos,
			"Please visit:\n"
			"https://www.bengreen.eu/?token=%s\n"
			"\n"
			"To see something.\n",
			token);
	bufpos += wlen;
	return buf;

error:
	if (buf != NULL) {
		free(buf);
	}
	return NULL;
}

int emailer_pipetomsmtp(char * buf) {
	/* From the perspective of the parent process */
	int wpipefd[2];
	int rpipefd[2];
	int buflen;
	int byteswritten;
	int pos;
	int forked;
	int res;

	wpipefd[0] = -1;
	wpipefd[1] = -1;
	rpipefd[0] = -1;
	rpipefd[1] = -1;

	res = pipe(wpipefd);
	check_debug(res >= 0, "Creation of Write pipe failed");
	res = pipe(rpipefd);
	check_debug(res >= 0, "Creation of Read pipe failed");

	forked = fork();
	check_debug(forked >= 0, "Fork failed");
	if (forked == 0) {
		// An Important note at this point, all the file descriptors of the
		// parent process have been dup()ed in this child.
		// That means we must close the parent end of the pipes
		close(wpipefd[1]);
		close(rpipefd[0]);
		// Now we need to move the file descriptor numbers from whatever
		// they are now to be the STDIN and STDOUT file descriptor numbers
		// i.e. 0 and 1 respectively.
		// We don't need the parent's STDIN and STDOUT, let's leave those
		// for our parent process.
		// This is not strictly necessary as the man page says dup2() will
		// silently close any file descriptor if it is open.
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		if (dup2(wpipefd[0], STDIN_FILENO) == -1) { fprintf(stderr, "ERROR dup2(wpipefd)\n"); return -1; }
		if (dup2(rpipefd[1], STDOUT_FILENO) == -1) { fprintf(stderr, "ERROR dup2(rpipefd)\n"); return -1; }
		// Now they are duplicated, close the "old" fds.
		close(wpipefd[0]);
		close(rpipefd[1]);
		char* const arguments[] = { "/usr/bin/msmtp", "--debug", "--account=bengreeneu", "--read-recipients", (char *)NULL };
		res = execv("/usr/bin/msmtp", arguments);
		// Remember that stdout is redirected... we should handle this better.
		fprintf(stderr, "Error: %d (%d)\n", res, errno);
		// When this call returns the process will be terminated
		// or should be!
		return -1;
	} else {
		// Don't need the child's end of the pipes.
		close(wpipefd[0]);
		close(rpipefd[1]);
		pos = 0;
		buflen = strlen(buf);
		do {
			byteswritten = write(wpipefd[1], buf + pos, buflen - pos);
			debug("Written : %d", byteswritten);
			if (byteswritten > 0) {
				pos += byteswritten;
			} else {
				break;
			}
		} while (pos < buflen);
		close(wpipefd[1]);
		debug("Written to pipe, forked = %d", forked);
		do {
			res = read(rpipefd[0], buf, buflen);
			if (res > 0) {
				buf[res] = '\0';
				debug("[%s]", buf);
			}
		} while (res > 0);
		close(rpipefd[0]);
		if (waitpid(forked, &res, 0) == 0) {
			debug("Exited? %d %d", WIFEXITED(res), WEXITSTATUS(res));
		}
		debug("Email sent to programme via pipe.\n");
	}
	return 0;

error:
	if (wpipefd[0] != -1) { close(wpipefd[0]); }
	if (wpipefd[1] != -1) { close(wpipefd[1]); }
	if (rpipefd[0] != -1) { close(rpipefd[0]); }
	if (rpipefd[1] != -1) { close(rpipefd[1]); }
	return -1;
}


