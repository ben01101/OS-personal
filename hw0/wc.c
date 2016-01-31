#include <stdio.h>

int main(int argc, char *argv[]) {
	if (argc > 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 2;
	}
	// fprintf(stdout, "%d\n", argc);
	int c;
	int prevC;
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
			if (isWhitespace(c) && !isWhitespace(prevC)) {
				wordCount++;
			}
			if (c == '\n') {
				lineCount++;
			}
		}
		fprintf(stdout, "\t%d %d %d\n", lineCount, wordCount, charCount);
		return 0;
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
		if (isWhitespace(c) && !isWhitespace(prevC)) {
			wordCount++;
		}
		if (c == '\n') {
			lineCount++;
		}
		// fprintf(stdout, "%d\n", c);
		charCount++;
		prevC = c;
	}
	fclose(wcFile);
	fprintf(stdout, "%d %d %d %s\n", lineCount, wordCount, charCount, argv[1]);
	return 0;
}

int isWhitespace(int c) {
	if (c == ' ' || c == '\n' || c == '\t' || c == '\0' || c == '\a' || c == '\b' || c == '\f' || c == '\r' || c == '\v') {
		return 1;
	}
	return 0;
}
