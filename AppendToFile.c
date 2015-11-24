#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<ctype.h>
#include<dirent.h>
#include<libgen.h>
#include<unistd.h>
#include<signal.h>

static volatile int run = 1;

void intHandler(int temp) {
	run = 0;
}

typedef struct node {
	char *name;
	struct node *next;
	struct node *prev;
} node;




int quotedString(char *str) {
	int len = strlen(str);
	if(str[0] == '"') {
		if(str[len-1] == '"') {
			return 1;
		}else {
			return -1;
		}
	}else if(str[0] == '\'') {
		if(str[len-1] == '\'') {
			return 1;
		}else {
			return -1;
		}
	}
	
	return 0;
}

char *removeQuotes(char *str) {
	char *temp = (char *) malloc((strlen(str) - 1) * sizeof(char));
	strncpy(temp, str+1, strlen(str) - 1);
	temp[strlen(str)-2] = '\0';
	str = NULL;
	return temp;
}

char *parseQuotedString(char *str) {
	int i = 1, ord = -1;
	int len = strlen(str);
	for(i = 1; i<len; i++) {
		ord = (int) str[i];
		
		printf("ord: %d\n", ord);
	}
	return str;
}


char *parseString(char *str) {
	int i = 0;
	int ord = -1;
	while((ord = (int) str[i++]) != 0) {
		if(47 < ord && ord < 58) continue;
		if(64 < ord && ord < 91) continue;
		if(96 < ord && ord < 123) continue;
		switch(ord) {
			case 131 :
				continue;
			case 138 :
				continue;
			case 140 :
				continue;
			case 142 :
				continue;
			case 154 :
				continue;
			case 156 :
				continue;
			case 158 :
				continue;
			case 159 :
				continue;
			case 170 :
				continue;
			case 178 :
				continue;
			case 179:
				continue;
			case 185 :
				continue;
			case 186 :
				continue;
			default :
				break;
		}
		
		if(191 < ord && ord < 256) {
			if(ord != 215 && ord != 247) continue;
		}
		
		printf("Bad character with ordinal value: %d\n", ord);
		return NULL;
	}
	
	return str;
}


node *insertNode(node *currentNode, char *name) {
	printf("insert: %s\n", name);
	if(strncmp(name, "..", 3) == 0 && strlen(name) == 2) {
		if(currentNode == NULL) {
			currentNode = (node *) malloc(sizeof(node));
			currentNode->name = NULL;
			currentNode->next = NULL;
			currentNode->prev = NULL;
		}
		if(currentNode->prev == NULL) {
			return currentNode;
		}else {
			currentNode = currentNode->prev;
			currentNode->next = NULL;
		}
	}else if(strncmp(name, ".", 2) == 0 && strlen(name) == 1) {
		return currentNode;
	}else {
		node *nextNode = (node *) malloc(sizeof(struct node));
		nextNode->name = (char *) malloc((strlen(name) + 2) * sizeof(char));
		strncpy(nextNode->name, "/", 2);
		strncat(nextNode->name, name, strlen(name) + 1);
		
		if(currentNode == NULL) {
			nextNode->prev = NULL;
			nextNode->next = NULL;
			return nextNode;
		}
		
		nextNode->prev = currentNode;
		nextNode->next = NULL;
		currentNode->next = nextNode;
		currentNode = nextNode;
	}
	
	return currentNode;
}

int getPathSize(node *root) {
	int size = 0;
	while(root != NULL) {
		printf("name: %s\n", root->name);
		size += strlen(root->name);
		root = root->next;
	}
	
	printf("size: %d\n", size);
	return size;
}

char *parseData(char *data) {
	int isQuoted = quotedString(data);
	if(isQuoted == -1) {
		printf("Error: File Path string not closed\n");
		return NULL;
	}
	
	char *newData = NULL;
	if(isQuoted) {
		char *temp = removeQuotes(data);
		newData = parseQuotedString(temp);
		//free(temp);
	}else {
		newData = parseString(data);
	}
	
	return newData;
}


char *getValidFilePath(node *root) {
	char *currPath = get_current_dir_name();
	if(currPath == NULL) return NULL;
	int size = getPathSize(root);
	char *path = (char *) malloc((size+1) * sizeof(char));
	
	strncpy(path, root->name, strlen(root->name)+1);
	root = root->next;
	
	while(root != NULL) {
		strncat(path, root->name, strlen(root->name)+1);
		root = root->next;
	}
	
	printf("path: %s\n", path);
	char temp[strlen(path) + 1];
	strncpy(temp, path, strlen(path) + 1);
	char *requestDir = dirname(temp);
	
	printf("requestDir: %s,  curDir: %s\n", requestDir, currPath);
	if(strncmp(requestDir, currPath, strlen(currPath)+1) != 0) {
		if(strncmp(requestDir, "/tmp", 5) != 0 || strlen(requestDir) != 4) {
			printf("File path must be in the current directory or /tmp\n");
			free(currPath);
			free(path);
			return NULL;
		}
	}
	
	free(currPath);
	
	printf("Final path: %s\n", path);
	return path;
}

node *getCurrentPath() {
	node *currentNode = NULL;
	char *path = get_current_dir_name();
	char buffer[strlen(path)+1];
	strncpy(buffer, path, strlen(path)+1);
	free(path);
	
	char *token = strtok(buffer, "/");
	
	while(token != NULL) {
		currentNode = insertNode(currentNode, token);
		token = strtok(NULL, "/");
	}
	
	return currentNode;
}

void freePath(node *root) {
	while(root->next != NULL) {
		free(root->name);
		root = root->next;
		free(root->prev);
	}
	
	free(root->name);
	free(root);
}

void freePathReverse(node *root) {
	while(root->prev != NULL) {
		free(root->name);
		root = root->prev;
		free(root->next);
	}
	
	free(root->name);
	free(root);
}

node *collapseFilePath(char *path) {
	
	int isQuoted = quotedString(path);
	if(isQuoted == -1) {
		printf("Error: File Path string not closed\n");
		return NULL;
	}
	
	node *currentNode = NULL;
	printf("%c\n", path[0]);
	
	char *checkedStr = NULL;
	if(!isQuoted) {
		currentNode = getCurrentPath();
		path = (char *) realloc(path, strlen(path) + 8);
		if(path == NULL) {
			printf("ERROR: No memory left to append uni to file path\n");
			freePathReverse(currentNode);
			return NULL;
		}
		strncat(path, ".jg3538", 8);
		checkedStr = parseString(path);
		if(checkedStr == NULL) {
			freePathReverse(currentNode);
			return NULL;
		}
		currentNode = insertNode(currentNode, checkedStr);
	}else {
	
		if(path[1] != '/') {
			currentNode = getCurrentPath();
		}
	
		path = removeQuotes(path);
		path = (char *) realloc(path, strlen(path) + 8);
		if(path == NULL) {
			printf("ERROR: No memory left to append uni to file path\n");
			freePathReverse(currentNode);
			return NULL;
		}
		strncat(path, ".jg3538", 8);
		
		char *token = strtok(path, "/");
	
		while(token != NULL) {
			printf("string: %s\n", token);
			checkedStr = parseQuotedString(token);
		
			if(checkedStr == NULL) {
				freePathReverse(currentNode);
				free(path);
				return NULL;
			}
		
			printf("checked string: %s\n", checkedStr);
			currentNode = insertNode(currentNode, checkedStr);
		
			token = strtok(NULL, "/");
		}
		
		free(path);
	}

	while(currentNode->prev != NULL) {
		currentNode = currentNode->prev;
		printf("curnode: %s\n", currentNode->name);
	}
	
	return currentNode;
}


void freeArgs(char **args) {
	if(args[0] != NULL) free(args[0]);
	if(args[1] != NULL) free(args[1]);
	free(args);
}

char **getInput() {
	int size = 1, limit = 4096;
	int isQuoted = 0, quote = 0, argIndex = 0, escape = 0, space = 0;
	
	char **args = (char **) malloc(2 * sizeof(char *));
	args[0] = (char *) malloc(limit * sizeof(char));
	args[1] = (char *) malloc(limit * sizeof(char));
	if(args[0] == NULL || args[1] == NULL) {
		freeArgs(args);
		printf("ERROR: Could not assign memory for malloc\n");
		return NULL;
	}
	
	while(run) {
		
		int c = getchar();
		
		if(argIndex == 2) {
			if(c == 9 || c == 32) {
				continue;
			}
			
			if(c == 10) {
				return args;
			}else {
				freeArgs(args);
				printf("ERROR: Can only supply 2 strings per line\n");
				return NULL;
			}
		}
		
		if(size == 1) {
			printf("size 1\n");
			
			if(space) {
				if(c == 9 || c == 32) {
					space = 0;
					continue;
				}else {
					freeArgs(args);
					printf("There should be at least 1 space between file path and data\n");
					return NULL;
				}
			}
			if(c == 10) {
				freeArgs(args);
				return NULL;
			}
			if(c == 34) {
				printf("34 quote\n");
				args[argIndex][size-1] = (char) c;
				size++;
				isQuoted = 1;
				quote = 34;
				continue;
			}else if(c == 39) {
				printf("39 quote\n");
				args[argIndex][size-1] = (char) c;
				size++;
				isQuoted = 1;
				quote = 39;
				continue;
			}else if(c == 9 || c == 32) {
				continue;
			}
		}
		
		
		if(isQuoted) {
			printf("quoted\n");
			if(quote == c && !escape) {
				printf("end of quote\n");
				args[argIndex][size-1] = c;
				size++;
				args[argIndex++][size-1] = '\0';
				size = 1;
				isQuoted = 0;
				quote = 0;
				escape = 0;
				space = 1;
				continue;
			}
		}else {
			if(c == 9 || c == 32) {
				if(argIndex == 0) {
					printf("end of argument\n");
					args[argIndex++][size-1] = '\0';
					if(argIndex == 2) return args;
					size = 1;
					escape = 0;
					space = 0;
					continue;
				}
			}
			else if(c == 10) {
				if(argIndex == 0) {
					printf("ERROR: Bad input format\n");
					freeArgs(args);
					return NULL;
				}
				printf("end of argument\n");
				args[argIndex++][size-1] = '\0';
				if(argIndex == 2) return args;
				size = 1;
				escape = 0;
				space = 1;
				continue;
			}
		}
		
		printf("char: %c, ord: %d\n", c, c);
		
		args[argIndex][size-1] = (char) c;
		size++;
		
		if(size == limit) {
			limit += 4096;
			args[argIndex] = (char *) realloc(args[argIndex], limit);
			if(args[argIndex] == NULL) {
				freeArgs(args);
				printf("ERROR: Malloc failed, not enough space\n");
				return NULL;
			}
		}	
	}
	
	freeArgs(args);
	return NULL;
}


int main(void) {
	signal(SIGINT, intHandler);
	char *command = NULL, *filePath = NULL;
	char **args = NULL;
	node *root = NULL;
	fd_set rfds;
	struct timeval tv;
	int retval;
	
	while(run) {
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		int retval = select(1, &rfds, NULL, NULL, &tv);
		
		if(retval == -1) {
			printf("ERROR on input\n");
			return 0;
		}else if(retval == 0) {
			continue;
		}
		
		args = getInput();
		if(args == NULL) {
			printf("NULL args\n");
			continue;
		}
		
		printf("file path: %s\n", args[0]);
		printf("data: %s\n", args[1]);
		printf("idk\n");
		root = collapseFilePath(args[0]);
		
		if(root == NULL) {
			printf("Invalid file path\n");
			freeArgs(args);
			continue;
		}
		
		filePath = getValidFilePath(root);
		
		if(filePath == NULL) {
			printf("Invalid file path\n");
			freePath(root);
			freeArgs(args);
			continue;
		}
		
		char *data = parseData(args[1]);
		if(data == NULL) {
			printf("Invalid format for data\n");
			freePath(root);
			freeArgs(args);
			free(filePath);
			continue;
		}
		
		command = (char *) malloc(strlen(filePath) + strlen(data) + 10);
		if(command == NULL) {
			printf("Not enough memory to create the echo command\n");
		}else {
			strncpy(command, "echo ", 6);
			strncat(command, data, strlen(data)+1);
			strncat(command, " >> ", 5);
			strncat(command, filePath, strlen(filePath)+1);
			
			int result = system(command);
			if(result == -1) {
				printf("ERROR: Could not append to file\n");
			}
		}
		
		//free(data);
		free(command);
		free(filePath);
		freePath(root);
		freeArgs(args);
	}
    
	if(command != NULL) free(command);
	if(filePath != NULL) free(filePath);
	if(root != NULL) freePath(root);
	if(args != NULL) freeArgs(args);
    return 0;
}
