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

void sigHandler(int s) {
	endwin();
	quick_exit(0);
}

Mix_Music* music;
bool willloop = false;

void loop() {
	if(willloop) 
		Mix_PlayMusic(music, 0);
}

char **choices = {
};


int main(int argc, char** argv) {
	//printf("");
	SDL_Init(SDL_INIT_AUDIO);
	Mix_Init(MIX_INIT_MP3);
	DIR* dir;
	int choice = 0;
	dir = opendir(argv[1]);
	struct dirent *dircont;
	if(dir) {
		while((dircont = readdir(dir)) != NULL) {
			if(dircont->d_type == DT_REG ) {
				choice++;
				choices=realloc(choices, sizeof(char*)*(choice));
				choices[choice-1] = dircont->d_name;
			}
		}
	}
	ITEM** items;
	int c = 12;
	MENU* menu;
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
	char*playingSong = "(None)";
	refresh();
	int width = COLS/3, height = LINES/1.3, x = (COLS-width)/2, y = (LINES-height)/2;
	main = newwin(height, width, y, x);
	//stat = newwin(6, 50, LINES-6, 0);
	stat = newwin(4, half+2, LINES-4, 0);
	box(main, 0, 0);

	wrefresh(main);
	wrefresh(stat);
	//choice = SIZE_ARRAY(choices);
	items = calloc(choice+1, sizeof(ITEM*));
	for(int i = 0; i < choice; ++i)
        items[i] = new_item(choices[i], "");
	items[choice] = (ITEM *)NULL;

	menu = new_menu(items);

	set_menu_win(menu, main);
	set_menu_format(menu, height-2, 1);
	set_menu_sub(menu, derwin(main, 0, 0, 1, 0));
	set_menu_pad(menu, 1);

	char* mspath = 0;
	
	post_menu(menu);
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

	WINDOW* textbox;

	float msicdur;
	float pos;
	float absPos = -1;
	char name[50];
	char *shellcmd;
	Mix_HookMusicFinished(loop);
	start_color();
	use_default_colors();
	init_pair(1, COLOR_WHITE, COLOR_WHITE);
	init_pair(2, COLOR_GREEN, -1);
	while(true) {
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
		for(int j = 0; j<half; j++) {
			mvwprintw(stat, 2, 1+j, "_");
		}
		//mvwprintw(stat, 2, 1+pos*48, "#");
		wattron(stat, COLOR_PAIR(1));
		mvwprintw(stat, 2, 1+pos*half, "#");
		wattroff(stat, COLOR_PAIR(1));
		if(strlen(playingSong) < half) {
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
			
			if(scrollamnt > strlen(playingSong)-(half-4)){
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
			strncpy(scrollcname, playingSong+scrollamnt, half-5);
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


	switch(c) {
			case 's': 
				menu_driver(menu, REQ_DOWN_ITEM);
				break;
			case 'w': 
				menu_driver(menu, REQ_UP_ITEM);
				break;

			case  KEY_DOWN: 
				menu_driver(menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP: 
				menu_driver(menu, REQ_UP_ITEM);
				break;

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
			case ' ': 
				if(Mix_PausedMusic()) {
					Mix_ResumeMusic();
				} else {
					Mix_PauseMusic();
				}
				break;
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

			case 'l':
				willloop = !willloop;
				break;
			default:
			break;
		}

		refresh();
		}
//	}
	free_menu(menu);
	endwin();
}
