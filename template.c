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
#include "template.h"

static int print_variable (FILE *in, FILE *out, pvar_list const *list)
{
	char buffer[TEMPLATE_VARLENGTH];
	char *bptr = buffer;
	int c;

	/* read variable name */
	c = fgetc(in);
	while (c != EOF && c != '%' && bptr < buffer + sizeof(buffer) - 1) {
		*bptr++ = c;
		c = fgetc(in);
	}
	*bptr = '\0';

	/* found a possible variable name? */
	if (c == '%') {
		if (bptr == buffer) {
			fputc('%', out);
			return -1;
		}

		/* attempt to find and print variable */
		while (list) {
			char const *output = list->callback(buffer, list->data);
			if (output) {
				fputs(output, out);
				return 0;
			}

			list = list->next;
		}
		return 0;
	}

	/* invalid variable name */
	fputc('%', out);
	fwrite(buffer, bptr - buffer, 1, out);
	if (c != EOF)
		fputc(c, out);
	return -1;
}

int template_print (char const *filename, FILE *out, pvar_list const *list)
{
	FILE *in;
	int c;

	/* open template file */
	in = fopen(filename, "r");
	if (!in)
		return -1;
	

	/* parse template file */
	c = fgetc(in);
	while (c != EOF) {
		if (c == '%')
			print_variable(in, out, list);
		else
			fputc(c, out);

		c = fgetc(in);
	}

	/* close template file */
	if (ferror(in)) {
		fclose(in);
		return -1;
	}
	return fclose(in);
}
