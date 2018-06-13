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

#ifndef _TEMPLATE_H
#define _TEMPLATE_H

#include <stdio.h>

#define TEMPLATE_VARLENGTH	256

typedef struct pvar_list_s {
	struct pvar_list_s *next;
	void *data;
	char const *(*callback)(char const *var, void const *data);
} pvar_list;

int template_print (char const *filename, FILE *out, pvar_list const *list);

#endif /* _TEMPLATE_H */
