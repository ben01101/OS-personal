#include <stdio.h>

int main(int argc, char *argv[]) {
	if (argc > 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 2;
	}
	// fprintf(stdout, "%d\n", argc);
	int c;
	int charCount = 0;
	int wordCount = 0;   
	int lineCount = 0;
	if (argc == 1) { 
		c = getchar();
		if (c != ' ' && c != '\n') {
			wordCount++;
		}
		while (c != EOF) {
			c = getchar();
			charCount++;
			if (c == ' ' || c == '\n') {
				wordCount++;
			}
			if (c == '\n') {
				lineCount++;
			}
		}
		fprintf(stdout, "\t%d %d %d\n", lineCount, wordCount, charCount);
		return;
	}
	FILE* wcFile = fopen(argv[1], "r");
	if (wcFile == NULL) {
		fprintf(stderr, "File not found: \"%s\"\n", argv[1]);
		return 2;
	}
	while (1) {
		c = fgetc(wcFile);
		if (feof(wcFile)) {
			break;
		}
		if (c == ' ' || c == '\n') {
			wordCount++;
		}
		if (c == '\n') {
			lineCount++;
		}
		// fprintf(stdout, "%d\n", c);
		charCount++;
	}
	fclose(wcFile);
	fprintf(stdout, "%d %d %d %s\n", lineCount, wordCount, charCount, argv[1]);
}
