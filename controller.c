#include <stdio.h>
#include <stdlib.h>

/* Module interfaces */
#include "iauthentication.h"

int
main(int argc, char **argv)
{
	char user[MAX_LEN_USER];
	char password[MAX_LEN_PW];

	/* ---------------- */
	/* just for testing */
	printf("user: ");
	scanf("%s", user);
	printf("password: ");
	scanf("%s", password);
	/* just for testing */
	/* ---------------- */

	printf("%d\n", signUp(user, password));
	return EXIT_SUCCESS;
}
