#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <menu.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE_ARRAY(a) (sizeof(a)/sizeof(a[0]))

void sigHandler(int s) {
	endwin();
	quick_exit(0);
}

char **choices = {
};


int main(int argc, char** argv) {
	Mix_Music* music;
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
	pthread_t thread;
	signal(SIGQUIT, sigHandler);
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, true);
	WINDOW *main;
	WINDOW *stat;
	nodelay(stdscr, true);
	char*playingSong = calloc(1, 1);
	refresh();
	int width = COLS/3, height = LINES/1.3, x = (COLS-width)/2, y = (LINES-height)/2;
	main = newwin(height, width, y, x);
	stat = newwin(6, 50, LINES-6, 0);
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

	char* mspath;
	
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
	
	while(true) {
		framecount++;
		werase(stat);
		wborder(stat, 0,0,0,0, 0,0,0,0);
		if(strlen(playingSong) < 48) {
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
			
			if(scrollamnt > strlen(playingSong)-47){
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
			strncpy(scrollcname, playingSong+scrollamnt, 47);
			mvwprintw(stat, 1, 1, "%s", scrollcname);

		}
		c = getch();
		wrefresh(stat);
		//if(c!=ERR) {
		attron(A_ITALIC);
		mvprintw(0, 0, "Stopify");
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

			case 'a': 
				break;
			case 'd': 
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
				break;
			case ' ': 
				if(Mix_PausedMusic()) {
					Mix_ResumeMusic();
				} else {
					Mix_PauseMusic();
				}
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
