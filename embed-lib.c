/*
et-sdl-sound - SDL sound system for Enemy Territory, RTCW and Q3
Copyright (C) 2007-2008  Pyry Haulos

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 6) {
		printf("%s [source script] [target script] [string] [binary] [mode]\n", argv[0]);
		return 1;
	}
	
	int mode = 0;
	
	if (!strcmp(argv[5], "nowrap"))
		mode = 0;
	else if (!strcmp(argv[5], "wrap"))
		mode = 1;
	
	long script_in_len, script_out_len, binary_len, binary_p_len;
	FILE *script_in_f = NULL;
	FILE *script_out_f = NULL;
	FILE *binary_f = NULL;
	
	if ((script_in_f = fopen(argv[1], "r")) == NULL)
		return 1;
	
	if ((binary_f = fopen(argv[4], "r")) == NULL)
		return 1;
	
	fseek(script_in_f, 0L, SEEK_END);
	script_in_len = ftell(script_in_f);
	fseek(script_in_f, 0L, SEEK_SET);
	
	fseek(binary_f, 0L, SEEK_END);
	binary_len = ftell(binary_f);
	fseek(binary_f, 0L, SEEK_SET);
	
	if (mode == 1) {
		// TODO check this
		binary_p_len = (binary_len / 19) * 4 + binary_len * 4;
		if (binary_len % 19 != 0)
			binary_p_len += 4;
	} else
		binary_p_len = binary_len * 4;
	
	script_out_len = script_in_len - strlen(argv[3]) + binary_p_len;
	
	char *script_in = (char *) malloc(binary_len);
	char *binary = (char *) malloc(binary_len);
	char *script_out = (char *) malloc(script_out_len);
	
	fread((void *) script_in, 1, script_in_len, script_in_f);
	fread((void *) binary, 1, binary_len, binary_f);
	
	fclose(script_in_f);
	fclose(binary_f);
	
	unsigned long str_s = (unsigned long) strstr(script_in, argv[3]);
	unsigned long str_e = str_s + (unsigned long) strlen(argv[3]);
	
	if (str_s == 0)
		return 1;
	
	memcpy((void *) script_out, (void *) script_in, str_s - (unsigned long) script_in);
	memcpy((void *) ((unsigned long) script_out + str_s - (unsigned long) script_in + (unsigned long) binary_p_len), (void *) str_e, script_in_len - (str_e - (unsigned long) script_in));
	
	unsigned long addr = (unsigned long) script_out + str_s - (unsigned long) script_in;
	
	unsigned int i;
	for (i = 0; i < binary_len; i++) {
		if (mode == 1 && (i % 19) == 0) {
			sprintf((char *) addr, "'\\\n'");
			addr += 4;
			// printf("\n");
		}
		
		// printf("0x%016x: 0x%02x\n", addr, (unsigned char) binary[i]);
		sprintf((char *) addr, "\\x%02x", (unsigned char) binary[i]);
		addr += 4;
	}
	*((char *) addr) = *((char *) str_e);
	
	if ((script_out_f = fopen(argv[2], "w")) == NULL)
		return 1;
	
	fwrite((void *) script_out, 1, script_out_len, script_out_f);
	fclose(script_out_f);
	
	free(script_in);
	free(script_out);
	free(binary);
	
	return 0;
}
