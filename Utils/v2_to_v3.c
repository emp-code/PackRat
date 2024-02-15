#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "../packrat.h"
#include "packrat_v2_read.h"

// packrat-v2_to_v3: Import files from a Pack Rat v2 archive to a v3 archive

static int printHelp(const char * const cmd) {
	printf("Pack Rat v2-to-v3 tool. Imports files from a v2 archive, and adds them to a v3 archive.\nUsage: %s --old-data=old.prd --old-index=old.pri --new-data=new.prd --new-index=new.pri\n", cmd);
	return 0;
}

int main(int argc, char *argv[]) {
	char *old_prd = NULL;
	char *old_pri = NULL;
	char *new_prd = NULL;
	char *new_pri = NULL;

	for(;;) {
		static struct option longOpts[] = {
			{"old-data",    required_argument, NULL, 'a'},
			{"old-index",   required_argument, NULL, 'b'},
			{"new-data",    required_argument, NULL, 'x'},
			{"new-index",   required_argument, NULL, 'y'},
			{0, 0, 0, 0}
		};

		const int c = getopt_long(argc, argv, "", longOpts, NULL);
		if (c == -1) break;

		switch (c) {
			case 'a': old_prd = optarg; break;
			case 'b': old_pri = optarg; break;
			case 'x': new_prd = optarg; break;
			case 'y': new_pri = optarg; break;

			default: return printHelp(argv[0]);
		}
	}

	if (old_prd == NULL || old_pri == NULL || new_prd == NULL || new_pri == NULL) return printHelp(argv[0]);

	int fileNum = 0;
	for(;;) {
		unsigned char *data = NULL;
		int lenData = packrat_v2_read(old_pri, old_prd, fileNum, &data);

		if (lenData == PACKRAT_V2_ERROR_ID) break; // End reached

		if (lenData < 0) {
			bool br = false;

			switch (lenData) {
				case PACKRAT_V2_ERROR_END: br = true; break;
				case PACKRAT_V2_ERROR_FILESIG: puts("Error: File signature does not match (not a Pack Rat archive?)"); br = true; break;
				case PACKRAT_V2_ERROR_CORRUPT: puts("Error: Index file corrupted"); br = true; break;
				case PACKRAT_V2_ERROR_OPEN: puts("Error: Failed to open file"); br = true; break;
				case PACKRAT_V2_ERROR_READWRITE: puts("Error: Failed to read file"); br = true; break;
				case PACKRAT_V2_ERROR_MISC: puts("Error: Misc error"); br = true; break;

				case PACKRAT_V2_ERROR_EMPTY: lenData = 0; break;
				case PACKRAT_V2_ERROR_ALLOC: printf("Skipping file #%d: failed allocation (too big?)\n", fileNum); lenData = 0; break;
				default: printf("Skipping file #%d: Unexpected error: %d\n", fileNum, lenData); lenData =  0;
			}

			if (br) break;
		}

		int ret = packrat_add(new_pri, new_prd, data, lenData);
		if (data != NULL) free(data);

		if (ret == PACKRAT_ERROR_TOOBIG) {
			printf("Skipping file #%d, too big\n", fileNum);
			ret = packrat_add(new_pri, new_prd, NULL, 0);
		}

		if (ret != PACKRAT_OK) {
			printf("Error adding file to new archive: %d\n", ret);
			break;
		}

		fileNum++;
		if (fileNum % 100000 == 0) printf("Progress: %.1fm files copied.\n", (double)fileNum / 1000000); // Reports every 0.1m
	}

	printf("%d files copied to new archive.\n", fileNum	);
	return 0;
}
