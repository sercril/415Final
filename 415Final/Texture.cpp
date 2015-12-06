#include <iostream>
#include <stdlib.h>
#include <string.h>

errno_t err;

/* LoadPPM - a minimal Portable Pixelformat file loader
fname: name of file to load (input)
w: width of loaded image in pixels (output)
h: height of loaded image in pixels (output)
data: image data address (input or output depending on mallocflag)
mallocflag: 1 if memory not pre-allocated, 0 if data already points
to allocated memory that can hold the image.
Note that if new memory is allocated, free() should be
used to deallocate when it is no longer needed.
*/

void LoadPPM(char *fname, unsigned int *w, unsigned int *h, unsigned char **data, int mallocflag){
	FILE *fp;
	char P, num;
	int max;
	char s[1000];

	printf("Reading image file: %s\n", fname);

	err = fopen_s(&fp, fname, "rb");

	if (err != 0){
		perror("cannot open image file\n");
		exit(0);
	}

	fscanf_s(fp, "%c%c\n", &P, 1, &num, 1);

	if ((P != 'P') || (num != '6')){
		perror("unknown file format for image\n");
		exit(0);
	}

	do{
		fgets(s, 999, fp);
	} while (s[0] == '#');

	sscanf_s(s, "%d%d", w, h);
	fgets(s, 999, fp);
	sscanf_s(s, "%d", &max);

	if (mallocflag)
	if (!(*data = (unsigned char *)malloc(*w * *h * 3))){
		perror("cannot allocate memory for image data\n");
		exit(0);
	}
	fread(*data, 3, *w * *h, fp);
	fclose(fp);
}