#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <menu.h>
#include <dirent.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>

#define SIZE_ARRAY(a) (sizeof(a)/sizeof(a[0]))


char* readfile(FILE *f) {
  // f invalid? fseek(m array of the given menu. ) fail?
  if (f == NULL || fseek(f, 0, SEEK_END)) {
    return NULL;
  }

  long length = ftell(f);
  rewind(f);
  // Did ftell() fail?  Is the length too long?
  if (length == -1 || (unsigned long) length >= SIZE_MAX) {
    return NULL;
  }

  // Convert from long to size_t
  size_t ulength = (size_t) length;
  char *buffer = (char*)malloc(ulength + 1);
  // Allocation failed? Read incomplete?
  if (buffer == NULL || fread(buffer, 1, ulength, f) != ulength) {
    free(buffer);
    return NULL;
  }
  buffer[ulength] = '\0'; // Now buffer points to a string

  return buffer;
}

struct playlist {
	char* name;
	/*
	MENU* menu;
	ITEM** items;
	*/
	char* dirname;
};


void sigHandler(int s) {
	endwin();
	quick_exit(0);
}


Mix_Music* music;
bool willloop = false;
char **queue;
int queueLen = 0;
MENU* menu;
char*playingSong = "(None)";
char* mspath = 0;

char* argv1;

float msicdur;
float pos;


void loop() {
	if(willloop) 
		Mix_PlayMusic(music, 0);
	else {
		if(queueLen <= 0) {
			queueLen = 0;
			return;
		}
				queueLen--;
				playingSong=malloc(strlen(queue[0])+1);
				strcpy(playingSong, queue[0]);
				mspath = malloc(strlen(argv1)+strlen(queue[0])+2);
				strcpy(mspath, argv1);
				strcat(mspath, "/");
				strcat(mspath, queue[0]);
				//printw("%s", mspath);
				music = Mix_LoadMUS(mspath);
				Mix_PlayMusic(music, 0);
				msicdur = Mix_MusicDuration(music);
				pos = Mix_GetMusicPosition(music)/msicdur;
	/*	for(int i = 1; i<queueLen+1; i++) {
			queue[i-1] = queue[i];
		}
*/
		memmove(queue, queue+1, queueLen*sizeof(char*));
		queue = realloc(queue, (queueLen)*sizeof(char*)+1);
	}
}

char **choices = {
};


int main(int argc, char** argv) {
	argv1 = argv[1];
	queue=malloc(sizeof(char*));
	//printf("");
	SDL_Init(SDL_INIT_AUDIO);
	Mix_Init(MIX_INIT_MP3);
	DIR* dir;
	int choice = 0;
	if(access(strcat(strdup(argv1), ".plists"), F_OK)) {
		FILE*f;
		f = fopen(strcat(strdup(argv1), ".plists"), "w");
		fclose(f);
	}
	if(access(strcat(strdup(argv1), ".play"), F_OK)) {
		FILE*f;
		f = fopen(strcat(strdup(argv1), ".play"), "w");
		fclose(f);
	}
	struct dirent *dircont;
	dir = opendir(argv[1]);
	if(dir) {
		while((dircont = readdir(dir)) != NULL) {
			if(dircont->d_type == DT_REG || dircont->d_type == DT_LNK && strcmp(dircont->d_name, ".plists")) {
				choice++;
				choices=realloc(choices, sizeof(char*)*(choice));
				choices[choice-1] = dircont->d_name;
			}
		}
	}
	ITEM** items;
	int c = 12;
	ITEM * cur_it;
	signal(SIGQUIT, sigHandler);
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, true);
	WINDOW *main;
	WINDOW *stat;
	nodelay(stdscr, true);
	int half = COLS/3-2;
	refresh();
	//int width = COLS/3, height = LINES/1.3, x = (COLS-width)/2, y = (LINES-height)/2;
	int width = COLS/3, height = LINES, x = (COLS-half)-COLS/6-1 , y = 0;
	main = newwin(height, width, y, x);
	//stat = newwin(6, 50, LINES-6, 0);
	stat = newwin(4, /*half+2*/ x, LINES-4, 0);
	box(main, 0, 0);

	wrefresh(main);
	wrefresh(stat);
	//choice = SIZE_ARRAY(choices);
	items = calloc(choice+1, sizeof(ITEM*));
	ITEM **dMenuItems = calloc(5, sizeof(ITEM*));
	MENU* mMenu;
	dMenuItems[0] = new_item("All", "");
	dMenuItems[1] = new_item("Liked", "");
	dMenuItems[2] = new_item("Playlists", "");
	dMenuItems[3] = new_item("Exit", "");
	dMenuItems[4] = NULL;
	for(int i = 0; i < choice; ++i)
        items[i] = new_item(choices[i], "");
	items[choice] = (ITEM *)NULL;

	char*plistn;
	FILE*fptr;
	FILE*playptr;
	char* fpath = malloc(7 + strlen(argv1));
	strcpy(fpath, argv1);
	strcat(fpath, ".plists");
	char*playpath = malloc(6+strlen(argv1));
	strcpy(playpath, argv1);
	strcat(playpath, ".play");
	bool pname = true;

	fptr = fopen(fpath, "r");
	playptr = fopen(playpath, "r");
	plistn = readfile(fptr);

	menu = new_menu(items);

	fseek(playptr, 0L, SEEK_END);
	int plsize = ftell(playptr);
	rewind(playptr);
	char* pline = malloc(plsize);



	mMenu = new_menu(dMenuItems);
	int plistcount = 0;
	ITEM** tempItems = calloc(1, sizeof(ITEM*));
	int titemscnt = 0;
	struct playlist* playlists = malloc(sizeof(struct playlist)*(plistcount));

while (fgets(pline, plsize, playptr)) {
//	printw("%s", pline);
//	refresh();
//	sleep(1);
    pline[strcspn(pline, "\n")] = '\0'; // safely remove newline
	if(pname) {
		plistcount++;
		playlists = realloc(playlists, sizeof(struct playlist) * plistcount);
		playlists[plistcount-1].name = strdup(pline);

	} else {
		playlists[plistcount-1].dirname = strdup(pline);

	}
	pname = !pname;
}

/*
	plistcount++;
	playlists = realloc(playlists, sizeof(struct playlist) * plistcount);
	playlists[plistcount-1].name = "undertale";
	playlists[plistcount-1].dirname = ".undertale";

	plistcount++;
	playlists = realloc(playlists, sizeof(struct playlist) * plistcount);
	playlists[plistcount-1].name = "mario";
	playlists[plistcount-1].dirname = ".mario";
	*/
	MENU* playlistNames = malloc(sizeof(MENU));
	ITEM** pnameItems = calloc(2, sizeof(ITEM*));
	for(int i = 0; i<plistcount; i++) {
	pnameItems = realloc(pnameItems, sizeof(ITEM*)*(i+1)+1);
	pnameItems[i] = new_item(strdup(playlists[i].name), "");
	pnameItems[i+1] = NULL;
	}
	playlistNames = new_menu(pnameItems);

	set_menu_win(playlistNames, main);
	set_menu_format(playlistNames, height-2, 1);
	set_menu_sub(playlistNames, derwin(main, 0, 0, 1, 0));
	set_menu_pad(playlistNames, 1);

	set_menu_win(menu, main);
	set_menu_format(menu, height-2, 1);
	set_menu_sub(menu, derwin(main, 0, 0, 1, 0));
	set_menu_pad(menu, 1);

	set_menu_win(mMenu, stdscr);
	set_menu_format(mMenu, 10, 1);
	set_menu_sub(mMenu, derwin(stdscr, 0, 0, 3, 0));
	set_menu_pad(mMenu, 1);

	
	post_menu(menu);
	post_menu(mMenu);
	wrefresh(main);
	refresh();

	if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
	{
		//SDL_Log("Unable to start SDL2_Mixer: %s", SDL_GetError());
		return 1;
	}

//	if(music == NULL) {
//		//SDL_Log("Unable to load music: %s", SDL_GetError());
//		return -1;
//	}

	int scrollamnt = 0;
	char scrollcname[48];
	bool goingright = true;
	int sat = 1;
	int framecount = 0;
	bool ends = false;
	int count = 0;
	char* songnamecut;

	WINDOW* textbox;

	WINDOW* queueView;
	queueView = newwin( LINES, half/2, 0,COLS-half/2);

	MENU*likeSongs;
	ITEM**likeSongsItems = calloc(4*sizeof(ITEM*), 1);

	MENU*currentPlaylist;
	ITEM**CplayItems = malloc(sizeof(ITEM*));


	fseek(fptr, 0L, SEEK_END);
	int filesize = ftell(fptr);
	rewind(fptr);
	char* line = malloc(filesize);
	int likeSongCount = 0;
	/*
	while(fgets(line, filesize, fptr)) {
		likeSongCount++;
		likeSongsItems = realloc(likeSongsItems, (likeSongCount)*(sizeof(ITEM*)));
line[strcspn(line, "\n")] = '\0';
		likeSongsItems[likeSongCount] = new_item(line, "");

	for(int i = 0; i<likeSongCount; i++) {
		printw("%s\n", item_name(likeSongsItems[i]));
		refresh();
		sleep(1);
	}
	}*/
while (fgets(line, filesize, fptr)) {
	// here to 4 lines below is generated by chatGPT because new_item is fucking stupid (i know why it needs strdup, but WHY did they make it NEED strdup?????)
    line[strcspn(line, "\n")] = '\0'; // safely remove newline
    likeSongCount++;
    likeSongsItems = realloc(likeSongsItems, likeSongCount * sizeof(ITEM*));
    likeSongsItems[likeSongCount - 1] = new_item(strdup(line), "");
}


	likeSongsItems[likeSongCount] = NULL;
	likeSongs = new_menu(likeSongsItems);


	set_menu_win(likeSongs, main);
	set_menu_format(likeSongs, height-2, 1);
	set_menu_sub(likeSongs, derwin(main, 0, 0, 1, 0));
	set_menu_pad(likeSongs, 1);


	float absPos = -1;
	int curMenu = 0;
	char name[50];
	char *shellcmd;

	bool inplist = false;

	Mix_HookMusicFinished(loop);
	start_color();
	use_default_colors();
	init_pair(1, COLOR_WHITE, COLOR_WHITE);
	init_pair(2, COLOR_GREEN, -1);
	int brkloop = false;
	while(true) {
	LINES = getmaxy(stdscr);
	COLS = getmaxx(stdscr);
		if(willloop) {
			mvprintw(1, 0, "Looping    ");
		} else {
			mvprintw(1, 0, "Not Looping");
		}
		if(Mix_PlayingMusic()) {
		absPos = Mix_GetMusicPosition(music);
		}
		if(absPos != -1) {
		pos = absPos/msicdur;
		}
				
		framecount++;
		werase(stat);
		wborder(stat, 0,0,0,0, 0,0,0,0);
	//	mvwprintw(stat, 2, 1, "________________________________________________");
		for(int j = 0; j<x-2; j++) {
			mvwprintw(stat, 2, 1+j, "_");
		}
		//mvwprintw(stat, 2, 1+pos*48, "#");
		wattron(stat, COLOR_PAIR(1));
		mvwprintw(stat, 2, 1+pos*(x-2), "#");
		wattroff(stat, COLOR_PAIR(1));
		if(strlen(playingSong) < x) {
			ends = false;
				mvwprintw(stat, 1, 1, "%s", playingSong);
		} else {
			/*
		if(!ends) {
				ends = true;
				playingSong = realloc(playingSong, strlen(playingSong)+7);
				memmove(playingSong+3, playingSong, strlen(playingSong)+4);
				playingSong[0] = ' ';
				playingSong[1] = ' ';
				playingSong[2] = ' ';
				strcat(playingSong, "   ");
			
			}*/
			//usleep(100000);
			
			if(scrollamnt > strlen(playingSong)-(x-4)){
			if(!ends) {
				ends = true;
				count = 0;
			}
			if(count > 5) {
				ends = false;
			} else {
				count++;
			}
				sat=-1;
			}
			if(scrollamnt <1){
			if(!ends) {
				ends = true;
				count = 0;
			}
			if(count > 5) {
				ends = false;
			} else {
				count++;
			}

				sat=1;
			}
			if(framecount > 2000) {
				if(!ends) {
					scrollamnt+=sat;
				}
				framecount = 0;
			}
			strncpy(scrollcname, playingSong+scrollamnt, x-5);
			scrollcname[x-5] = '\0';
			mvwprintw(stat, 1, 1, "%s", scrollcname);

		}
		c = getch();
		wrefresh(stat);
		//if(c!=ERR) {
		attron(A_ITALIC);
		attron(COLOR_PAIR(2));
		mvprintw(0, 0, "Stopify");
		attroff(COLOR_PAIR(2));
		attroff(A_ITALIC);
		

		box(main, 0, 0);
		wrefresh(main);

		if(c == '\t') {
			curMenu = !curMenu;
		}
		if(c == '.') {
			loop();
		}

		switch(c) {

			case KEY_LEFT:
			case 'a': 
				if(Mix_PlayingMusic()) {
					Mix_SetMusicPosition(fmin(fmax(absPos-5, 0), msicdur));
				} else {
				if(mspath == 0) break;

				music = Mix_LoadMUS(mspath);
				Mix_PlayMusic(music, 0);
				Mix_SetMusicPosition(fmax(msicdur-5, 0));


				}
				break;
			case KEY_RIGHT:
			case 'd': 
				Mix_SetMusicPosition(fmin(fmax(absPos+5, 0), msicdur));
				break;
			case 'l':
				willloop = !willloop;
				break;
			case ' ': 
				if(Mix_PausedMusic()) {
					Mix_ResumeMusic();
				} else {
					Mix_PauseMusic();
				}
				break;
			case 'm':
				curMenu = 0;
				break;

			default:
				break;

		}


	if(!strcmp(item_name(current_item(mMenu)), "All") && curMenu) {
	switch(c) {
			case 's': 
				menu_driver(menu, REQ_DOWN_ITEM);
				break;
			case 'w': 
				menu_driver(menu, REQ_UP_ITEM);
				break;

			case 'h' /* h for heart <3*/
				/* fuck that heart shit i got too many crashes h is for hatred*/
				:
				
				rewind(fptr);
			
			/*	for(int i = 0; i<likeSongCount; i++) {
					//if(!strcmp(item_name(current_item(menu)), item_name(likeSongsItems[i]))) {
					//	brkloop = true;
					//}
				}*/
			/*	if(brkloop) {
					brkloop = false;
					break;
				}*/
			//	likeSongCount++;
   			//	likeSongsItems = realloc(likeSongsItems, likeSongCount * sizeof(ITEM*));
   			//	likeSongsItems[likeSongCount - 1] = current_item(menu);
			//	likeSongsItems[likeSongCount] = NULL;
				char* argdup = malloc(strlen(argv1));
				argdup = strdup(argv1);
				FILE*lksongsfptr = fopen(strcat(strdup(argv1), ".plists"), "a");
				fprintf(lksongsfptr, "%s\n", item_name(current_item(menu)));
				fflush(lksongsfptr);
				fclose(lksongsfptr);

	likeSongsItems = calloc(4*sizeof(ITEM*), 1);
	fseek(fptr, 0L, SEEK_END);
	int filesize = ftell(fptr);
	rewind(fptr);
	char* line = malloc(filesize);
	int likeSongCount = 0;
while (fgets(line, filesize, fptr)) {
	// here to 4 lines below is generated by chatGPT because new_item is fucking stupid (i know why it needs strdup, but WHY did they make it NEED strdup?????)
    line[strcspn(line, "\n")] = '\0'; // safely remove newline
    likeSongCount++;
    likeSongsItems = realloc(likeSongsItems, likeSongCount * sizeof(ITEM*));
    likeSongsItems[likeSongCount - 1] = new_item(strdup(line), "");
}


	likeSongsItems[likeSongCount] = NULL;
	likeSongs = new_menu(likeSongsItems);


	set_menu_win(likeSongs, main);
	set_menu_format(likeSongs, height-2, 1);
	set_menu_sub(likeSongs, derwin(main, 0, 0, 1, 0));
	set_menu_pad(likeSongs, 1);
			//unpost_menu(likeSongs);
			//set_menu_items(likeSongs, likeSongsItems);
				break;


			case 'q':
				queueLen++;
				queue = realloc(queue, queueLen*sizeof(char*)+1);


				queue[queueLen-1] = malloc(strlen(item_name(current_item(menu)))+1);
				queue[queueLen-1] = (char*)item_name(current_item(menu));
				break;
			case  KEY_DOWN: 
				menu_driver(menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP: 
				menu_driver(menu, REQ_UP_ITEM);
				break;

			/*
			case KEY_LEFT:
			case 'a': 
				if(Mix_PlayingMusic()) {
					Mix_SetMusicPosition(fmin(fmax(absPos-5, 0), msicdur));
				} else {
				if(mspath == 0) break;

				music = Mix_LoadMUS(mspath);
				Mix_PlayMusic(music, 0);
				Mix_SetMusicPosition(fmax(msicdur-5, 0));


				}
				break;
			case KEY_RIGHT:
			case 'd': 
				Mix_SetMusicPosition(fmin(fmax(absPos+5, 0), msicdur));
				break;
			*/
			case '\n':
				playingSong=malloc(strlen(item_name(current_item(menu)))+1);
				strcpy(playingSong, item_name(current_item(menu)));
				mspath = malloc(strlen(argv[1])+strlen((item_name(current_item(menu))))+2);
				strcpy(mspath, argv[1]);
				strcat(mspath, "/");
				strcat(mspath, item_name(current_item(menu)));
				//printw("%s", mspath);
				music = Mix_LoadMUS(mspath);
				Mix_PlayMusic(music, 0);
				msicdur = Mix_MusicDuration(music);
				pos = Mix_GetMusicPosition(music)/msicdur;
				break;
				/*
			case ' ': 
				if(Mix_PausedMusic()) {
					Mix_ResumeMusic();
				} else {
					Mix_PauseMusic();
				}
				break;
				*/
			case 'c':
			/*	textbox = newwin(3, 40, LINES/2-2, COLS/2-20);
				box(textbox, 0, 0);
				wrefresh(textbox);
				//nodelay();
				echo();
				//mvwscanw(textbox,1,1, "");
				mvwgetnstr(textbox,1,1,name,39);
				noecho();
				shellcmd = malloc(39+6+1+2+1+strlen(item_name(current_item(menu)))+1+strlen(argv[1])*2+1+1+1+12);
				strcpy(shellcmd, "ffmpeg -i ");
				strcat(shellcmd, argv[1]);
				strcat(shellcmd, "/");
				strcat(shellcmd, item_name(current_item(menu)));
				strcat(shellcmd, " ");
				strcat(shellcmd, argv[1]);
				strcat(shellcmd, "/");
				strcat(shellcmd, name);
				strcat(shellcmd, " &> /dev/null");
				system(shellcmd);
				delwin(textbox);


				goto start;

*/
				// converts but doesnt fit the program
				break;

				/*
			case 'l':
				willloop = !willloop;
				break;
				*/
			default:
			break;
		}
		} else if(!strcmp(item_name(current_item(mMenu)), "Liked") && curMenu) {
			switch(c) {
				case KEY_DOWN:
				case 's':
					menu_driver(likeSongs, REQ_DOWN_ITEM);
					break;
				case KEY_UP:
				case 'w':
					menu_driver(likeSongs, REQ_UP_ITEM);
					break;

				case '\n':
					playingSong=malloc(strlen(item_name(current_item(likeSongs)))+1);
					strcpy(playingSong, item_name(current_item(likeSongs)));
					mspath = malloc(strlen(argv[1])+strlen((item_name(current_item(likeSongs))))+2);
					strcpy(mspath, argv[1]);
					strcat(mspath, "/");
					strcat(mspath, item_name(current_item(likeSongs)));
					//printw("%s", mspath);
					music = Mix_LoadMUS(mspath);
					Mix_PlayMusic(music, 0);
					msicdur = Mix_MusicDuration(music);
					pos = Mix_GetMusicPosition(music)/msicdur;
					break;
			case 'q':
				queueLen++;
				queue = realloc(queue, queueLen*sizeof(char*)+1);


				queue[queueLen-1] = malloc(strlen(item_name(current_item(likeSongs)))+1);
				queue[queueLen-1] = (char*)item_name(current_item(likeSongs));
				break;

			case 'r':
				rewind(fptr);
				unpost_menu(likeSongs);
				FILE*lksongsfptr = fopen(strcat(strdup(argv1), ".plists"), "w");
				fprintf(lksongsfptr, "");
				/*lksongsfptr = fopen(strcat(strdup(argv1), ".plists"), "a");
				char*printtofile = malloc(filesize*2);
				for(int i = 0; i<likeSongCount; i++) {
					//if(strcmp(item_name(likeSongsItems[i]), item_name(current_item(likeSongs)))) {
					if(i!=0){
					strcat(printtofile, item_name(likeSongsItems[i]));
					strcat(printtofile, "\n");
					} else {
					strcpy(printtofile, item_name(likeSongsItems[i]));
					strcat(printtofile, "\n");
					}
				printw("%s", strdup(item_name(likeSongsItems[i])));
				refresh();
				sleep(1);

		//	}
				}
				printw("%s", printtofile);
				refresh();
				sleep(1);
				fprintf(lksongsfptr, "%s", printtofile);
				*/
				fflush(lksongsfptr);
				fclose(lksongsfptr);

	likeSongsItems = calloc(4*sizeof(ITEM*), 1);
	fseek(fptr, 0L, SEEK_END);
	int filesize = ftell(fptr);
	rewind(fptr);
	char* line = malloc(filesize);
	int likeSongCount = 0;
while (fgets(line, filesize, fptr)) {
	// here to 4 lines below is generated by chatGPT because new_item is fucking stupid (i know why it needs strdup, but WHY did they make it NEED strdup?????)
    line[strcspn(line, "\n")] = '\0'; // safely remove newline
    likeSongCount++;
    likeSongsItems = realloc(likeSongsItems, likeSongCount * sizeof(ITEM*));
    likeSongsItems[likeSongCount - 1] = new_item(strdup(line), "");
}


	likeSongsItems[likeSongCount] = NULL;
	likeSongs = new_menu(likeSongsItems);


	set_menu_win(likeSongs, main);
	set_menu_format(likeSongs, height-2, 1);
	set_menu_sub(likeSongs, derwin(main, 0, 0, 1, 0));
	set_menu_pad(likeSongs, 1);







				post_menu(likeSongs);
				break;

				default:
					break;
			}
		}

	else if(!strcmp(item_name(current_item(mMenu)), "Playlists") && curMenu) {
		if(!inplist) {
		switch(c) {

				case KEY_DOWN:
				case 's':
					menu_driver(playlistNames, REQ_DOWN_ITEM);
					break;
				case KEY_UP:
				case 'w':
					menu_driver(playlistNames, REQ_UP_ITEM);
					break;

				case '\n':
				strcpy(line, argv1);
				strcat(line, "/");
				strcat(line, playlists[item_index(current_item(playlistNames))].dirname);
				CplayItems = malloc(0);

	dir = opendir(line);
	int count = 0;
	if(dir) {
		while((dircont = readdir(dir)) != NULL) {
			if(dircont->d_type == DT_LNK && strcmp(dircont->d_name, ".plists")) {
				count++;
				mvprintw(1, 0, "%lu", sizeof(ITEM*)*(count)+1);
				refresh();
				usleep(300000);
				CplayItems=realloc(CplayItems, sizeof(ITEM*)*(count)+1);
				CplayItems[count-1] = new_item(dircont->d_name, "");
				CplayItems[count] = NULL;
			}
		}
	}
					currentPlaylist = new_menu(CplayItems);
	set_menu_win(currentPlaylist, main);
	set_menu_format(currentPlaylist, height-2, 1);
	set_menu_sub(currentPlaylist, derwin(main, 0, 0, 1, 0));
	set_menu_pad(currentPlaylist, 1);
					werase(main);
					post_menu(currentPlaylist);

					inplist = true;
					break;

				default:
					break;
		}
		
	} else {

		switch(c) {

				case KEY_DOWN:
				case 's':
					menu_driver(currentPlaylist, REQ_DOWN_ITEM);
					break;
				case KEY_UP:
				case 'w':
					menu_driver(currentPlaylist, REQ_UP_ITEM);
					break;

				case '\n':
				playingSong=malloc(strlen(item_name(current_item(currentPlaylist)))+1);
				strcpy(playingSong, item_name(current_item(currentPlaylist)));
				mspath = malloc(strlen(argv[1])+strlen((item_name(current_item(currentPlaylist))))+2);
				strcpy(mspath, argv[1]);
				strcat(mspath, "/");
				strcat(mspath, item_name(current_item(currentPlaylist)));
				//printw("%s", mspath);
				music = Mix_LoadMUS(mspath);
				Mix_PlayMusic(music, 0);
				msicdur = Mix_MusicDuration(music);
				pos = Mix_GetMusicPosition(music)/msicdur;
					break;
			case 'q':
				queueLen++;
				queue = realloc(queue, queueLen*sizeof(char*)+1);


				queue[queueLen-1] = malloc(strlen(item_name(current_item(currentPlaylist)))+1);
				queue[queueLen-1] = (char*)item_name(current_item(currentPlaylist));
				break;
			case 'h' /* h for heart <3*/
				/* fuck that heart shit i got too many crashes h is for hatred*/
				:
				
				rewind(fptr);
			
			/*	for(int i = 0; i<likeSongCount; i++) {
					//if(!strcmp(item_name(current_item(menu)), item_name(likeSongsItems[i]))) {
					//	brkloop = true;
					//}
				}*/
			/*	if(brkloop) {
					brkloop = false;
					break;
				}*/
			//	likeSongCount++;
   			//	likeSongsItems = realloc(likeSongsItems, likeSongCount * sizeof(ITEM*));
   			//	likeSongsItems[likeSongCount - 1] = current_item(menu);
			//	likeSongsItems[likeSongCount] = NULL;
				char* argdup = malloc(strlen(argv1));
				argdup = strdup(argv1);
				FILE*lksongsfptr = fopen(strcat(strdup(argv1), ".plists"), "a");
				fprintf(lksongsfptr, "%s\n", item_name(current_item(currentPlaylist)));
				fflush(lksongsfptr);
				fclose(lksongsfptr);

	likeSongsItems = calloc(4*sizeof(ITEM*), 1);
	fseek(fptr, 0L, SEEK_END);
	int filesize = ftell(fptr);
	rewind(fptr);
	char* line = malloc(filesize);
	int likeSongCount = 0;
while (fgets(line, filesize, fptr)) {
	// here to 4 lines below is generated by chatGPT because new_item is fucking stupid (i know why it needs strdup, but WHY did they make it NEED strdup?????)
    line[strcspn(line, "\n")] = '\0'; // safely remove newline
    likeSongCount++;
    likeSongsItems = realloc(likeSongsItems, likeSongCount * sizeof(ITEM*));
    likeSongsItems[likeSongCount - 1] = new_item(strdup(line), "");
}


	likeSongsItems[likeSongCount] = NULL;
	likeSongs = new_menu(likeSongsItems);


	set_menu_win(likeSongs, main);
	set_menu_format(likeSongs, height-2, 1);
	set_menu_sub(likeSongs, derwin(main, 0, 0, 1, 0));
	set_menu_pad(likeSongs, 1);
			//unpost_menu(likeSongs);
			//set_menu_items(likeSongs, likeSongsItems);
				break;

				default:
					break;
		}
		
	}

	}
	else {
			switch(c) {
				case KEY_DOWN:
				case 's':
					menu_driver(mMenu, REQ_DOWN_ITEM);
					break;
				case KEY_UP:
				case 'w':
					menu_driver(mMenu, REQ_UP_ITEM);
					break;
				case '\n':
					if(!strcmp(item_name(current_item(mMenu)), "Exit")) {
						endwin();
						return 0;
					} else if(!strcmp(item_name(current_item(mMenu)), "All")) {
						curMenu = !curMenu;
					}
					else if(!strcmp(item_name(current_item(mMenu)), "Liked")) {
						curMenu = !curMenu;
					}
					else if(!strcmp(item_name(current_item(mMenu)), "Playlists")) {
						curMenu = !curMenu;
					}

					break;
				default:
					break;
			}
		unpost_menu(menu);
		unpost_menu(likeSongs);
		unpost_menu(playlistNames);
		inplist = false;
		//unpost_menu(currentPlaylist);

		if(!strcmp(item_name(current_item(mMenu)), "All")) {
			unpost_menu(menu);
			unpost_menu(likeSongs);
			post_menu(menu);
		}else if(!strcmp(item_name(current_item(mMenu)), "Liked")){
			unpost_menu(menu);
			unpost_menu(likeSongs);
			post_menu(likeSongs);
		} else if(!strcmp(item_name(current_item(mMenu)), "Playlists")) {
			unpost_menu(menu);
			unpost_menu(likeSongs);
			if(!inplist) {
				post_menu(playlistNames);
			} else {
				post_menu(currentPlaylist);
			}
		//	mvwprintw(main, 1, 1, "hi");
		} else {
			unpost_menu(menu);
			mvwprintw(main, 1, 1, "Dont leave :(");
			unpost_menu(likeSongs);
		}
		}
		werase(queueView);
		songnamecut = malloc(half/2-2);
		for(int i = 0; i<queueLen; i++) {
			strncpy(songnamecut, queue[i], half/2-2);
			mvwprintw(queueView, 1+i, 1, "%s", songnamecut);
		}
		box(queueView, 0, 0);
		mvwprintw(queueView, 0, 1, "Queue:");
		wrefresh(queueView);

		refresh();
		}
//	}
	free_menu(menu);
	endwin();
}
