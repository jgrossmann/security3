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

//flag for input loop. 
static volatile int run = 1;

//sigint callback
void intHandler(int temp) {
	run = 0;
}

//node in linked list for filepath
typedef struct node {
	char *name;
	struct node *next;
	struct node *prev;
} node;



/*
Returns:
	 1 if the string is properly quoted.
	 0 if the string is not quoted.
	 -1 if the string is not properly quoted.
*/
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


/*
	Removes surrounding quotes from string
*/
char *removeQuotes(char *str) {
	char *temp = (char *) malloc((strlen(str) - 1) * sizeof(char));
	strncpy(temp, str+1, strlen(str) - 1);
	temp[strlen(str)-2] = '\0';
	str = NULL;
	return temp;
}


/*
	Parses quoted string, fixing escapes, and testing to see
	if all input is legal. 
*/
char *parseQuotedString(char *str) {
	int i = 0, ord = -1, subtract = 0;
	int len = strlen(str), escape = 0, octal = 0;
	char *newStr = (char *) malloc((strlen(str) + 1) * sizeof(char));
	if(newStr == NULL) {
		printf("ERROR: Not enough memory to malloc\n");
		return NULL;
	}
	for(i = 0; i<len; i++) {
		ord = (int) str[i];
		if(escape) {
		
			if(octal > 0) {
				if(47 < ord && ord < 58) {
					octal++;
					if(octal == 3) {
						octal = 0;
						escape = 0;
						char temp[] = {str[i-2], str[i-1], str[i], '\0'};
						char *end;
						int val = (int) strtol(temp, &end, 8);
						if(val == 0) {
							printf("ERROR: Null character is not allowed as input\n");
							free(newStr);
							return NULL;
						}
						subtract+= 3;
						newStr[i-subtract] = (char) val;
					}
					continue;
				}else {
					int k = 0;
					char temp[octal+1];
					for(;k<octal;k++) {
						temp[k] = str[i-octal+1+k];
					}
					temp[k] = '\0';
					char *end;
					int val = (int) strtol(temp, &end, 8);
					if(val == 0) {
						printf("ERROR: Null character not allowed as input\n");
						free(newStr);
						return NULL;
					}
					subtract += octal;
					newStr[i-subtract] = (char) val;
					octal = 0;
				}
			}else if(47 < ord && ord < 58) {
				octal++;
				continue;
			}else if(ord == 110) {
				subtract++;
				newStr[i-subtract] = (char) 10;
			}else if(ord == 114) {
				subtract++;
				newStr[i-subtract] = (char) 13;
			}else if(ord == 116) {
				subtract++;
				newStr[i-subtract] = (char) 9;
			}else if(ord == 34 || ord == 92) {
				newStr[i-subtract-1] = (char) 92;
				newStr[i-subtract] = (char) ord;
			}else if(ord == 39) {
				subtract++;
				newStr[i-subtract] = (char) ord;
			}else {
				printf("ERROR: Unrecognized escape character\n");
				free(newStr);
				return NULL;
			}
				
			escape = 0;
			continue;
		}else if(ord == 92) {
			escape = 1;
			continue;
		}else {
			escape = 0;
		}

		newStr[i-subtract] = str[i];
	}
	newStr[i-subtract] = '\0';
	return newStr;
}


/*
	Parses non quoted strings and checks if all characters
	all legal.
*/
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
		
		printf("ERROR: Bad character with ordinal value: %d\n", ord);
		return NULL;
	}
	
	return str;
}


/*
	Inserts new node into filepath linked list designated by
	the string name.
*/
node *insertNode(node *currentNode, char *name) {
	if(currentNode == NULL) {
		currentNode = (node *) malloc(sizeof(node));
		currentNode->name = (char *) malloc(2 * sizeof(char));
		strncpy(currentNode->name, "/", 2);
		currentNode->next = NULL;
		currentNode->prev = NULL;
	}
	
	if(strncmp(name, "..", 3) == 0 && strlen(name) == 2) {
		if(currentNode->prev == NULL) {
			return currentNode;
		}else {
			currentNode = currentNode->prev;
			free(currentNode->next->name);
			free(currentNode->next);
			currentNode->next = NULL;
		}
	}else if(strncmp(name, ".", 2) == 0 && strlen(name) == 1) {
		return currentNode;
	}else {
		node *nextNode = (node *) malloc(sizeof(struct node));
		if(strncmp(currentNode->name, "/", 2) == 0 && strlen(currentNode->name) == 1) {
			nextNode->name = (char *) malloc((strlen(name) + 1) * sizeof(char));
			strncpy(nextNode->name, name, strlen(name)+1);
		}else {
			nextNode->name = (char *) malloc((strlen(name) + 2) * sizeof(char));
			strncpy(nextNode->name, "/", 2);
			strncat(nextNode->name, name, strlen(name) + 1);
		}
		
		
		nextNode->prev = currentNode;
		nextNode->next = NULL;
		currentNode->next = nextNode;
		currentNode = nextNode;
	}
	
	return currentNode;
}

/*
	Returns size of path as a string by combining
	all names in file path linked list.
*/
int getPathSize(node *root) {
	int size = 0;
	while(root != NULL) {
		size += strlen(root->name);
		root = root->next;
	}
	
	return size;
}


/*
	Parses the data string to make sure it is valid
*/
char *parseData(char *data) {
	int isQuoted = quotedString(data);
	if(isQuoted == -1) {
		printf("Error: Data string not closed\n");
		return NULL;
	}
	
	char *newData = NULL;
	char *temp = NULL;
	if(isQuoted) {
		temp = removeQuotes(data);
		newData = parseQuotedString(temp);
		return newData;
	}else {
		newData = parseString(data);
		if(newData == NULL) {
			return NULL;
		}
		temp = (char *) malloc((strlen(newData) + 1) * sizeof(char));
		strncpy(temp, newData, strlen(newData) + 1);
		return temp;
	}
}


/*
	Returns the file path to append data to if the path is valid.
	Returns NULL if file path is not valid
*/
char *getValidFilePath(node *root) {
	char *currPath = getcwd(NULL, 0); //get_current_dir_name();
	if(currPath == NULL) {
		printf("ERROR: Could not get current path\n");
		return NULL;
	}
	int size = getPathSize(root);
	char *path = (char *) malloc((size+1) * sizeof(char));
	if(path == NULL) {
		printf("ERROR: Not enough memory to malloc\n");
		return NULL;
	}
	
	strncpy(path, root->name, strlen(root->name)+1);
	root = root->next;
	
	while(root != NULL) {
		strncat(path, root->name, strlen(root->name)+1);
		root = root->next;
	}
	
	char temp[strlen(path) + 1];
	strncpy(temp, path, strlen(path) + 1);
	char *requestDir = dirname(temp);
	
	if(strncmp(requestDir, currPath, strlen(currPath)+1) != 0) {
		if(strncmp(requestDir, "/tmp", 5) != 0 || strlen(requestDir) != 4) {
			printf("ERROR: File path must be in the current directory or /tmp\n");
			free(currPath);
			free(path);
			return NULL;
		}
	}
	
	free(currPath);
	return path;
}

/*
	Creates a linked list of the current file path.
	Each node is a directory
*/
node *getCurrentPath() {
	node *currentNode = NULL;
	char *path = getcwd(NULL, 0); //get_current_dir_name();
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

//Cleans up linked list from root
void freePath(node *root) {
	while(root->next != NULL) {
		free(root->name);
		root = root->next;
		free(root->prev);
	}
	
	free(root->name);
	free(root);
}

//Cleans up linked list from tail
void freePathReverse(node *root) {
	while(root->prev != NULL) {
		free(root->name);
		root = root->prev;
		free(root->next);
	}
	
	free(root->name);
	free(root);
}

/*
	Takes a string path and collapses it to find the
	absolute path without "." or ".."
*/
node *collapseFilePath(char *path) {
	
	int isQuoted = quotedString(path);
	if(isQuoted == -1) {
		printf("Error: File Path string not closed\n");
		return NULL;
	}
	
	node *currentNode = NULL;
	
	char *checkedStr = NULL;
	if(!isQuoted) {
		currentNode = getCurrentPath();
		checkedStr = parseString(path);
		if(checkedStr == NULL) {
			freePathReverse(currentNode);
			return NULL;
		}
		
		char *temp = (char *) malloc(strlen(checkedStr) + 8);
		if(temp == NULL) {
			printf("ERROR: No memory left to append uni to file path\n");
			freePathReverse(currentNode);
			return NULL;
		}
		strncpy(temp, checkedStr, strlen(checkedStr) + 1);
		strncat(temp, ".jg3538", 8);
		currentNode = insertNode(currentNode, temp);
		free(temp);
	}else {
	
		if(path[1] != '/') {
			currentNode = getCurrentPath();
		}
	
		path = removeQuotes(path);
		char temp[strlen(path)+1];
		strncpy(temp, path, strlen(path)+1);
		char *base = basename(temp);
		if(strncmp(base, "..", 3) == 0 && strlen(base) == 2) {
			path = (char *) realloc(path, strlen(path) + 9);
			if(path == NULL) {
				printf("ERROR: No memory left to append uni to file path\n");
				freePathReverse(currentNode);
				return NULL;
			}
			strncat(path, "/.jg3538", 8);
		}else {
			path = (char *) realloc(path, strlen(path) + 8);
			if(path == NULL) {
				printf("ERROR: No memory left to append uni to file path\n");
				freePathReverse(currentNode);
				return NULL;
			}
			strncat(path, ".jg3538", 8);
		}
		
		char *token = strtok(path, "/");
	
		while(token != NULL) {
			checkedStr = parseQuotedString(token);
		
			if(checkedStr == NULL) {
				freePathReverse(currentNode);
				free(path);
				return NULL;
			}
		
			currentNode = insertNode(currentNode, checkedStr);
			free(checkedStr);
			token = strtok(NULL, "/");
		}
		
		free(path);
	}

	while(currentNode->prev != NULL) {
		currentNode = currentNode->prev;
	}
	
	return currentNode;
}

//frees argument list
void freeArgs(char **args) {
	if(args[0] != NULL) free(args[0]);
	if(args[1] != NULL) free(args[1]);
	free(args);
}


/*
	Gets input from stdin.
	Performs rudimentary parsing. Mostly just makes sure
	that the input is two strings where each string is either 
	correctly quoted or not quoted. More rigorous parsing is done right after this
	function.
*/
char **getInput() {
	int size = 1, limit = 4096;
	int isQuoted = 0, quote = 0, argIndex = 0, escape = 0, space = 0;
	int error = 0;
	
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
		if(error) {
			if(c == 10) {
				return NULL;
			}
			continue;
		}
		
		//if there are already two strings
		if(argIndex == 2) {
			if(c == 9 || c == 32) {
				continue;
			}
			
			if(c == 10) {
				return args;
			}else {
				freeArgs(args);
				printf("ERROR: Can only supply 2 strings per line\n");
				error = 1;
				continue;
			}
		}
		
		//beginning of a new string
		if(size == 1) {
			
			if(space) {
				if(c == 9 || c == 32) {
					space = 0;
					continue;
				}else {
					freeArgs(args);
					printf("ERROR: There should be at least 1 space between file path and data\n");
					error = 1;
					continue;
				}
			}
			if(c == 10) {
				if(argIndex > 0) {
					if(strncmp(args[0], "quit", 5) == 0) {
						freeArgs(args);
						exit(0);
					}
					printf("ERROR: You must give 2 strings\n");
				}
				freeArgs(args);
				return NULL;
			}
			if(c == 34) {
				args[argIndex][size-1] = (char) c;
				size++;
				isQuoted = 1;
				quote = 34;
				continue;
			}else if(c == 39) {
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
			if(quote == c && !escape) {
				args[argIndex][size-1] = c;
				size++;
				args[argIndex++][size-1] = '\0';
				size = 1;
				isQuoted = 0;
				quote = 0;
				escape = 0;
				space = 1;
				continue;
			}else if(quote != c && !escape &&(c == 34 || c == 39)) {
				printf("ERROR: Must end string with same quote marks\n");
				freeArgs(args);
				error = 1;
				continue;
			}
		}else {
			if(c == 9 || c == 32) {
				if(argIndex == 0) {
					args[argIndex++][size-1] = '\0';
					if(argIndex == 2) return args;
					size = 1;
					escape = 0;
					space = 0;
					continue;
				}else {
					space = 1;
					continue;
				}
			}
			else if(c == 10) {
				if(argIndex == 0) {
					args[0][size-1] = '\0';
					if(strncmp(args[0], "quit", 5) == 0) {
						freeArgs(args);
						exit(0);
					}
					printf("ERROR: Bad input format\n");
					freeArgs(args);
					return NULL;
				}
				args[argIndex++][size-1] = '\0';
				if(argIndex == 2) return args;
				size = 1;
				escape = 0;
				space = 1;
				continue;
			}else if(space) {
				printf("ERROR: You can only input 2 strings per line\n");
				freeArgs(args);
				error = 1;
				continue;
			}
		}
		
		args[argIndex][size-1] = (char) c;
		size++;
		
		if(c == 92) {
			escape = 1;
		}else {
			escape = 0;
		}
		
		if(size == limit) {
			limit += 4096;
			args[argIndex] = (char *) realloc(args[argIndex], limit);
			if(args[argIndex] == NULL) {
				freeArgs(args);
				printf("ERROR: Malloc failed, not enough space\n");
				error = 1;
				continue;
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
			printf("\nShutting Down\n\n");
			return 0;
		}else if(retval == 0) {
			continue;
		}
		
		args = getInput();
		if(args == NULL) {
			fflush(stdin);
			continue;
		}
		
		root = collapseFilePath(args[0]);
		
		if(root == NULL) {
			freeArgs(args);
			continue;
		}
		
		filePath = getValidFilePath(root);
		
		if(filePath == NULL) {
			freePath(root);
			freeArgs(args);
			continue;
		}
		
		char *data = parseData(args[1]);
		if(data == NULL) {
			freePath(root);
			freeArgs(args);
			free(filePath);
			continue;
		}
		
		command = (char *) malloc(strlen(filePath) + strlen(data) + 18);
		if(command == NULL) {
			printf("ERROR: Not enough memory to malloc the echo command\n");
		}else {
			strncpy(command, "echo ", 6);
			strncat(command, "\"", 3);
			strncat(command, data, strlen(data)+1);
			strncat(command, "\"", 3);
			strncat(command, " >> ", 5);
			strncat(command, "\"", 3);
			strncat(command, filePath, strlen(filePath)+1);
			strncat(command, "\"", 3);
			
			
			//printf("Command:\n");
			//printf("%s\n", command);
			int result = system(command);
			if(result == -1) {
				printf("ERROR: Could not append to file\n");
			}
			printf("SUCCESS\n");
		}
		
		free(data);
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
