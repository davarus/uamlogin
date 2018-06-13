/*
 * Copyright (c) 2008, David A. Russell
 * All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warrenty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restricitions:
 *
 *     * The origin of this software must not be misrepresented; you must not
 *       claim that you wrote the original software.  If you use this software
 *       in a product, an acknowledgement in the product documentation would be
 *       appreciated but is not required.
 *     * Altered source versions must be plainly marked as such, and must not
 *       be misrepresented as being the original software.
 *     * Neither the name of David A. Russell nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *     * This notice and copyright may not be removed or altered from any
 *       source distribution.
 */

#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include "md5.h"
#include "template.h"
#include "cgi.h"

char const *uam_secret = "supersecret";

/*
 * UAM GET VARIABLES
 *	res
 *		success
 *		failed
 *		logoff
 *		already
 *		notyet
 *		TODO: The following...
 *		smartclient
 *		popup1
 *		popup2
 *		popup3
 *	challenge
 *	uamip
 *	uamport
 *	reply
 *	userurl
 *	timeleft
 *	redirurl
 */

void hex_pack (void const *in_buf, void *out_buf, size_t len)
{
	const char *in = in_buf;
	unsigned char *out = out_buf;
	while (len--) {
		unsigned char high, low, final;

		/* get the high nyble */
		if (isdigit(*in))
			high = *in - '0';
		else
			high = tolower(*in) - 'a' + 10;
		in++;
		final = high << 4;

		/* handle short string */
		if (!*in) {
			*out = final;
			return;
		}

		/* get the low nyble */
		if (isdigit(*in))
			low = *in - '0';
		else
			low = tolower(*in) - 'a' + 10;

		/* done */
		final |= low;
		*out++ = final;
		in++;
	}
}

void hex_unpack (void const *in_buf, void *out_buf, size_t len)
{
	static char const * const hex = "0123456789ABCDEF";
	unsigned char const *in = in_buf;
	char *out = out_buf;
	while (len--) {
		unsigned char byte;
		byte = *in++;
		*out++ = hex[(byte >> 4) & 0xF];
		*out++ = hex[byte & 0xF];
	}
	*out = '\0';
}


char *uam_res = NULL;
char *uam_challenge = NULL;

char *uam_ip = NULL;
char *uam_port = NULL;

char *uam_userurl = NULL;
char *uam_redirurl = NULL;
char *uam_reply = NULL;
char *uam_timeleft = NULL;

char *username = NULL;
char *password = NULL;

void cgi_callback (char const *name, char *value)
{
	if (!strcmp("res", name))
		uam_res = value;
	else if (!strcmp("challenge", name))
		uam_challenge = value;
	else if (!strcmp("uamip", name))
		uam_ip = value;
	else if (!strcmp("uamport", name))
		uam_port = value;
	else if (!strcmp("userurl", name))
		uam_userurl = value;
	else if (!strcmp("redirurl", name))
		uam_redirurl = value;
	else if (!strcmp("reply", name))
		uam_reply = value;
	else if (!strcmp("timeleft", name))
		uam_timeleft = value;
	else if (!strcmp("username", name))
		username = value;
	else if (!strcmp("password", name))
		password = value;
}


char response[33] = "";
char pappassword[33] = "";
const char *query_userurl = NULL;
char const *query_redirurl = NULL;

pvar_list pvl;
char const *template_callback (char const *var, void const *data)
{
	(void)data;

	/* uam variables */
	if (!strcmp(var, "challenge"))
		return uam_challenge;
	else if (!strcmp(var, "uamip"))
		return uam_ip;
	else if (!strcmp(var, "uamport"))
		return uam_port;
	else if (!strcmp(var, "userurl"))
		return uam_userurl;
	else if (!strcmp(var, "redirurl"))
		return uam_redirurl;
	else if (!strcmp(var, "reply"))
		return uam_reply;
	else if (!strcmp(var, "timeleft"))
		return uam_timeleft;

	/* URL Encoded URLs */
	else if (!strcmp(var, "userurl_"))
		return query_userurl;
	else if (!strcmp(var, "redirurl_"))
		return query_redirurl;

	/* logon variables */
	else if (!strcmp(var, "username"))
		return username;
	else if (!strcmp(var, "password"))
		return pappassword;
	else if (!strcmp(var, "response"))
		return response;

	/* variable not found */
	return NULL;
}

int main (int argc, char **argv)
{
	(void)argc;
	(void)argv;

	memset(&pvl, 0, sizeof(pvl));
	pvl.callback = template_callback;

	puts("Content-type: text/html\n");

	/* NOTE: This leaks memory. */
	if (cgi_parse(512, cgi_callback)) {
		template_print("templates/error.html", stdout, &pvl);
		return 0;
	}

	/* NOTE: These leak memory. */
	if (uam_userurl)
		query_userurl = cgi_urlencode(uam_userurl);
	if (uam_redirurl)
		query_redirurl = cgi_urlencode(uam_redirurl);

	if (!uam_ip || !uam_port) {
		template_print("templates/error.html", stdout, &pvl);
		return 0;
	}

	if (uam_res) {
		
		if (!strcmp(uam_res, "success") || !strcmp(uam_res, "already")) {
			template_print("templates/success.html", stdout, &pvl);
			return 0;
		}

		if (!strcmp(uam_res, "logoff")) {
			template_print("templates/logoff.html", stdout, &pvl);
			return 0;
		}

		if (!strcmp(uam_res, "notyet")) {
			template_print("templates/main.html", stdout, &pvl);
			return 0;
		}

		if (!strcmp(uam_res, "failed")) {
			template_print("templates/failed.html", stdout, &pvl);
			return 0;
		}

		template_print("templates/error.html", stdout, &pvl);
		return 0;
	}

	if (username && uam_challenge) {
		md5_state_t pms;
		md5_byte_t chal_packed[16];
		md5_byte_t res_packed[16];
		unsigned char pap_packed[16];
		int i;

		/* challenge */
		hex_pack(uam_challenge, chal_packed, sizeof(chal_packed));
		if (uam_secret) {
			md5_init(&pms);
			md5_append(&pms, chal_packed, sizeof(chal_packed));
			md5_append(&pms, (md5_byte_t *)uam_secret, strlen(uam_secret));
			md5_finish(&pms, chal_packed);
		}

		/* response */
		md5_init(&pms);
		md5_append(&pms, (md5_byte_t *)"\0", 1);
		if (password && *password) {
			md5_append(&pms, (md5_byte_t *)password, strlen(password));
		}
		md5_append(&pms, chal_packed, sizeof(chal_packed));
		md5_finish(&pms, res_packed);
		hex_unpack(res_packed, response, sizeof(res_packed));

		/* password */
		if (password && *password) {
			int paslength;

			paslength = strlen(password);
			if (paslength > 16)
				paslength = 16;

			for (i = 0; i < paslength; i++)
				pap_packed[i] = chal_packed[i] ^ password[i];
			for (; i < 16; i++)
				pap_packed[i] = chal_packed[i] ^ 0;

			hex_unpack(pap_packed, pappassword, sizeof(pap_packed));
		}

		template_print("templates/login.html", stdout, &pvl);
		return 0;
	}

	template_print("templates/error.html", stdout, &pvl);
	return 0;
}
