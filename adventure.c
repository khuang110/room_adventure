#define  _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>


#define DIR_PREFIX "huangky.rooms."

/*
*	Array of names of the rooms
*/
char roomNames[10][9] = {
  "ZYZZZY",
  "PLUGH",
  "PLOVER",
  "twisty",
  "Zork",
  "Crowther",
  "Dungeon",
  "Flip",
  "BURRITO",
  "monkey"
};

/*
* array of file names
*/
char roomFiles[10][13] = {
  "ZYZZZY.txt",
  "PLUGH.txt",
  "PLOVER.txt",
  "twisty.txt",
  "Zork.txt",
  "Crowther.txt",
  "Dungeon.txt",
  "Flip.txt",
  "BURRITO.txt",
  "monkey.txt"
};

/*
* room struct that contains data for each room
*/
struct room
{
	char* roomName;
	char* roomType;
	int numConnections;
	// int array that references an index in roomNames array
	int* connections;
};

/*
* find the dir that was opened most recently
*/
char* find_latest_directory(char* path, char* prefix) {
    struct stat dirStat;
    char directoryName[256];
    char* latestDirName;

    // Open the directory
    DIR* currDir = opendir(path);
    struct dirent* aDir;
    time_t lastModifTime;
    int i = 0;

    /* The data returned by readdir() may be overwritten by subsequent calls to  readdir()  for  the  same  directory stream. So we will copy the name of the directory to the variable directoryName
    */
    while ((aDir = readdir(currDir)) != NULL) {
        // Use strncmp to check if the directory name matches the prefix
        if (strncmp(prefix, aDir->d_name, strlen(prefix)) == 0) {
            stat(aDir->d_name, &dirStat);
            // Check to see if this is the directory with the latest modification time
            if (i == 0 || difftime(dirStat.st_mtime, lastModifTime) > 0) {
                lastModifTime = dirStat.st_mtime;
                memset(directoryName, '\0', sizeof(directoryName));
                strcpy(directoryName, aDir->d_name);
            }
            i++;
        }
    }
    latestDirName = malloc(sizeof(char) * (strlen(directoryName) + 1));
    strcpy(latestDirName, directoryName);

    closedir(currDir);
    return latestDirName;
}


/*
* Create and return a string for the file path by concatenating the directory name with the file name 
*/
char* get_file_path(char* directoryName, char* fileName) {
    char* filePath = malloc(strlen(directoryName) + strlen(fileName) + 2);
    memset(filePath, '\0', strlen(directoryName) + strlen(fileName) + 2);
    strcpy(filePath, directoryName);
    strcat(filePath, "/");
    strcat(filePath, fileName);
    return filePath;
}

/*
* returns index of room name that was found
* char* buf is room name that is collected from file
*/
int get_position(char* buf)
{
	for (int i = 0; i < 10; i++)
	{
		if (strcmp(roomNames[i], buf) == 0)
		{
			return i;
		}
	}
	return -1;
}

/*
* process data in a file and fill room struct
*/
struct room* process_file(char* filePath) {
	// Open the specified file for reading only
	FILE* fp = fopen(filePath, "r");

	size_t len = 0;
	ssize_t nread;
	char buf[100];
	struct room* newRoom = malloc(sizeof(struct room*));
	int i = 0;
	newRoom->connections = (int *)calloc(6, sizeof(int));
	for (int i = 0; i < 6; i++)
		newRoom->connections[i] = -1;
	// Read all the lines in the file
	while (fscanf(fp, "%*s %*s %s ", buf) == 1) {	// ignore first 2 words in colum

		// check if on the last line of the file
		if (strcmp("START_ROOM", buf) == 0)
			break;
		else if (strcmp("MID_ROOM", buf) == 0)
			break;
		else if (strcmp("END_ROOM", buf) == 0)
			break;

		
		if (i == 0) // first line of file 
		{
			// set room name
			newRoom->roomName = malloc(strlen(buf) + 2);
			strcpy(newRoom->roomName, buf);
		}
		else  // not on first line of file
		{
			// set room connection
			newRoom->connections[i - 1] = get_position(buf);
		}
		i++;
	}
	newRoom->roomType = buf;
	newRoom->numConnections = i - 1;
	return newRoom;
}

int menu(struct room* currRoom)
{
	while (1)	// endless loop so user has to enter right room name
	{
		printf("\nCURRENT LOCATION: %s\n", currRoom->roomName);

		printf("POSSIBLE CONNECTIONS:");
		for (int i = 0; i < currRoom->numConnections; i++)
		{
			// change how output looks so it ends in period
			if (i < currRoom->numConnections - 1)
				printf(" %s,", roomNames[currRoom->connections[i]]);
			else
				printf(" %s.\n", roomNames[currRoom->connections[i]]);
		}
		char buf[30];

		printf("WHERE TO? >");
		scanf("%s", buf);	//get input

		for (int i = 0; i < currRoom->numConnections; i++)
		{
			// check if room is a valid name
			if (strcmp(roomNames[currRoom->connections[i]], buf) == 0)
			{
				return currRoom->connections[i];
			}
		}
		printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
	}
}

/*
* goes through the files and finds what one is the start room
*/
struct room* find_start_room(char* latestDir)
{
	struct room* r;
	char *filePath;
	int i = 0;
	DIR *d;
	struct dirent *dir;
	d = opendir(latestDir);
	
	if (d) {
		// if dir is open
		while ((dir = readdir(d)) != NULL) {
			// skip the . and .. dir names
			if (i == 0 || i == 1)
			{
				i++;
				continue;
			}
			filePath = get_file_path(latestDir, dir->d_name);
			r = process_file(filePath);
			free(filePath);
			// if the type is startroom then break loop
			if (strcmp(r->roomType, "START_ROOM") == 0)
			{
				break;
			}


		}
		closedir(d);
	}

	return r;
}

/*
* frees some stuff
*/
void free_all(struct room* r)
{
	free(r->roomName);
	free(r);
	r = NULL;
}

int main()
{
	int steps = 0,
		idx = 0,
		path[20] = { 0 };
    char* latestDir = find_latest_directory(".", DIR_PREFIX),
		*filePath;
	printf("Latest dir = %s\n", latestDir);
	struct room* r;
	r = find_start_room(latestDir);


	while (1)	// loop till user finds end
	{
		idx = menu(r);
		filePath = get_file_path(latestDir, roomFiles[idx]);
		r = process_file(filePath);
		path[steps] = idx;	// array of indexes that correspond to names

		if (strcmp(r->roomType, "END_ROOM") == 0)
		{
			printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
			printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);

			// print the path
			for (int j = 0; j < (steps); j++)
			{
				printf("%s\n", roomNames[path[j]]);
			}
			break;
		}
		else
		{
			steps++;
		}
	}

	return 0;
}