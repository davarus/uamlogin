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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "cgi.h"

char *cgi_urlencode (char const *in)
{
	char *out, *ret;
	const char *tmp;
	static char const * const hex_digits = "0123456789ABCDEF";
	size_t len = 0;

	/* calculate space requirements */
	tmp = in;
	while (*tmp) {
		if (isalnum(*tmp++))
			len++;
		else
			len += 3;
	}
	len++;

	/* allocate output string */
	out = malloc(len);
	if (!out)
		return NULL;
	ret = out;

	/* encode string */
	while (*in) {
		if (isalnum(*in)) {
			*out++ = *in++;
		} else if (*in == ' ') {
			/* TODO: This isn't right */
			*out++ = '+';
			in++;
		} else {
			*out++ = '%';
			*out++ = hex_digits[(*(unsigned char *)in >> 4) & 0xF];
			*out++ = hex_digits[*(unsigned char *)in & 0xF];
			in++;
		}
	}
	*out = '\0';

	/* done */
	return ret;
}

static unsigned char hex_digit (char const c)
{
	if (isdigit(c))
		return c - '0';
	else
		return tolower(c) - 'a' + 10;
}

static void cgi_urldecode (char *out, char const *in, void (*callback)(char const *name, char *value))
{
	char const *name;
	char *value;

	name = out;
	value = NULL;
	while (*in) {
		/* TODO: Use a switch? */
		if (*in == '&') {
			*out++ = '\0';
			callback(name, value);
			name = out;
			value = NULL;
			in++;
		} else if (*in == '=') {
			*out++ = '\0';
			value = out;
			in++;
		} else if (*in ==  '+') {
			*out++ = ' ';
			in++;
		} else if (*in == '%') {
			if (!in[1] || !in[2])
				break;

			*out++ = (hex_digit(in[1]) << 4) | hex_digit(in[2]);
			in += 3;
		} else {
			*out++ = *in++;
		}
	}

	*out = '\0';
	if (*name)
		callback(name, value);
}

static char *form_query = NULL;
static char *form_post = NULL;
int cgi_parse (long maxpost, void (*callback)(char const *name, char *value))
{	
	char *query;
	char *method;

	method = getenv("REQUEST_METHOD");
	if (!method)
		return -1;

	/* process get method */
	query = getenv("QUERY_STRING");
	if (query && *query)
	{
		form_query = malloc(strlen(query) + 1);
		if (!form_query)
			return -2;

		cgi_urldecode(form_query, query, callback);

		if (!strcmp(method, "GET"))
			return 0;
	}

	if (!strcmp(method, "POST")) {
		char *env;
		long len;

		env = getenv("CONTENT_TYPE");
		if (!env || strcmp(env, "application/x-www-form-urlencoded"))
			return -1;

		env = getenv("CONTENT_LENGTH");
		if (!env)
			return -1;
		len = atol(env);
		if (!len || len > maxpost)
			return -1;

		form_post = malloc(len + 1);
		if (!form_post)
			return -2;

		if (fread(form_post, len, 1, stdin) != (size_t)len) {
			return -1;
		}
		form_post[len] = '\0';

		cgi_urldecode(form_post, form_post, callback);

		return 0;
	}

	return -1;
}

void cgi_free (void)
{
	free(form_query);
	free(form_post);
	form_query = NULL;
	form_post = NULL;
}
