#ifndef EMAILER_H
#define EMAILER_H

#ifdef __cplusplus
extern "C" {
#endif

char * emailer_createreminder(char * emailaddress, char * token);
int emailer_pipetomsmtp(char * buf);

#ifdef __cplusplus
}
#endif

#endif // EMAILER_H

