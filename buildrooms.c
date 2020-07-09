#define  _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define DIR_NAME "huangky.rooms."
#define ROOM_COUNT 7

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
*	room types
*/
 char roomTypes[3][11] = {
	"START_ROOM",
	"MID_ROOM",
	"END_ROOM"
};


/*
* stack implemetation of links to link the rooms together
*/
struct link
{
	struct room* roomData;
	struct link* next;
};

/*
* struct for room data
*/
struct room
{
	char* roomType;
	char* roomName;
	int numLinks;
	struct link* links;
};

/*
* struct for list of rooms contains 7 
*/
struct rooms
{
	struct room* rooms[ROOM_COUNT];
};

/*
*	function to convert int to char* 
*/
char* itoa(int val, int base) {
	static char buffer[32] = { 0 };
	int i = 30;

	for (; val && i; --i, val /= base)
		buffer[i] = "0123456789"[val % base];

	return &buffer[i + 1];
}

/*
*	append .txt onto the end of a room name
*/
char* append_txt(const char* roomName)
{
	char* t = ".txt";

	char* fileName = malloc(strlen(roomName) + strlen(t) + 2);
	memset(fileName, '\0', strlen(roomName) + strlen(t) + 2);
	strcpy(fileName, roomName);
	strcat(fileName, t);
	return fileName;
}

/*
* fills a txt file with the room data in the correct format
*/
void write_file(struct room* room, char* roomNum) 
{
	FILE* fp;
	chdir(roomNum);
	char* fileName = append_txt(room->roomName);
	fp = fopen(fileName, "w");
	fprintf(fp, "ROOM NAME: %s\n", room->roomName);

	struct link* curr = room->links;

	// loop through num links to add all of the connections associated with the room
	for (int i = 0; i < room->numLinks; i++)
	{
		fprintf(fp, "CONNECTION %d: %s\n", (i + 1), curr->roomData->roomName);
		curr = curr->next;
	}
	fprintf(fp, "ROOM TYPE: %s\n", room->roomType);

	free(fileName);
	fclose(fp);
}


/*
* Links 2 rooms together
* Parameters: startingroom: room to add a link, targetRoom: room to send a link to
*/
void link_rooms(struct room* startRoom, struct room* targetRoom)
{
	struct link* temp = malloc(sizeof(struct link*));

	// add temp to the front of the list making it the new "head"
	temp->roomData = targetRoom;
	temp->next = startRoom->links;
	startRoom->links = temp;
	startRoom->numLinks++;
}

/*
* randomly add a links to the list
*/
void add_link(struct rooms* roomlist)
{
	int i = 0,
		j = 0;

	for (i = 0; i < ROOM_COUNT - 1; i++)
	{
		for (j = i; j < ROOM_COUNT; j++)
		{
			// if j is odd and i is even or if j is even and i is odd
			if ( (j % 2 == 1 && i % 2 == 0) || (j % 2 == 0 && i % 2 == 1) )
			{
				// add a 2 way connection
				link_rooms(roomlist->rooms[i], roomlist->rooms[j]);
				link_rooms(roomlist->rooms[j], roomlist->rooms[i]);
			}
		}
	}
}

/*
* function that creates an array of all the rooms. 
* fills the rooms with member data
* returns array of all of the rooms
*/
struct rooms* create_rooms()
{
	struct rooms* roomList = malloc( sizeof(struct rooms) );

	// temp array to store index associated with a name in the roomNames array
	int names[ROOM_COUNT],
		i = 0,
		idx;

	// set array of room name index to use
	start:
	do
	{
		idx = rand() % 10;

		// check array to see if name is already in it
		for (int j = 0; j < ROOM_COUNT; j++)
		{
			// if array already contains the name randomize and check for new name
			if (names[j] == idx)
				goto start;
		}
		names[i++] = idx;
	} while (i < ROOM_COUNT);

	for (i = 0; i < ROOM_COUNT; i++)
	{
		struct room* r = malloc( sizeof(struct room) );

		// add rooms to roomList array
		roomList->rooms[i] = r;

		// set room types
		if (i == 0)
			r->roomType = roomTypes[0];	// start room
		else if (i == 1)
			r->roomType = roomTypes[2];	// end room
		else
			r->roomType = roomTypes[1];	// mid room

		// set name to randomly chosen one in room name index array
		r->roomName = roomNames[names[i]];
		r->numLinks = 0;
		r->links = NULL;
	}

	return roomList;
}


/*
*	Create a roooms dir
*/
char* create_room_dir()
{
	// get rand number 0-99999 convert to char* and make a dir with the number as a name
	unsigned int randNum = (rand() % 100000);
	// convert int to char
	char* dirNum = (itoa(randNum, 10));


	char* dirName = malloc(strlen(DIR_NAME) + strlen(dirNum) + 2);
	memset(dirName, '\0', strlen(DIR_NAME) + strlen(dirNum) + 2);
	// append numbers onto huangky.rooms.
	strcpy(dirName, DIR_NAME);
	strcat(dirName, dirNum);

	mkdir(dirName, 0777);
	
	return dirName;
}

/*
* frees memory
*/
void free_all(struct rooms* roomList)
{
	// free all of the rooms in the array
	for (int i = 0; i < ROOM_COUNT; i++)
	{
		//free links for each room
		struct link* temp = roomList->rooms[i]->links;

		// go through stack and free the room
		while (temp != NULL)
		{
			free(temp);
			temp = temp->next;
		}
		free(roomList->rooms[i]);
	}
	// free array last
	free(roomList);
}

int main(void)
{
	// seed time
	srand((unsigned)time(NULL));
	struct rooms* roomList = create_rooms();
	add_link(roomList);
	char* roomNum = create_room_dir();

	// create all the room files
	for (int i = 0; i < ROOM_COUNT; i++)
	{
		write_file(roomList->rooms[i], roomNum);
	}
	free_all(roomList);
	free(roomNum);

	return 0;
}
