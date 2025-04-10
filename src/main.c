#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <curses.h>
#include <menu.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

typedef struct {
	WINDOW *main;
	WINDOW *sub;
	MENU *menu;
	ITEM **item;
} Context;

typedef enum {
	ITEM_INVALID = 0,
	ITEM_START,
	ITEM_EXIT
} MenuChoice;

char *menu_labels[] = {
	"Start",
	"Exit"
};

typedef enum {
	STATE_MENU,
	STATE_PLAYING,
	STATE_PAUSE,
	STATE_GAME_OVER,
	STATE_EXIT
} GameState;

typedef struct {
	int y;
	int x;
} Vector2;

typedef struct Node {
	struct Node *prev;
	struct Node *next;
	Vector2 position;
} Node;

typedef struct {
	Node *head;
	Node *tail;
	char body;
	Vector2 direction;
} Snake;

typedef struct {
	Vector2 position;
	char body;
} Apple;

MenuChoice label_to_choice(const char *label) {
    if (strcmp(label, "Start") == 0) {
        return ITEM_START;
    }
    else if (strcmp(label, "Exit") == 0) {
        return ITEM_EXIT;
    }
    return ITEM_INVALID;
}

int main(void) {
	/* Seed rand */
	srand(time(NULL));

	/* Initialise ncurses */
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);

	/* Create main window */
	int height = 20, width = height * 2;
	int starty = (LINES - height) / 2;
	int startx = (COLS - width) / 2;
	WINDOW *window = newwin(height, width, starty, startx);
	keypad(window, TRUE);
	nodelay(window, TRUE);
	wrefresh(window);

	/* Create sub window */ 
	int sub_height = height - 3, sub_width = width - 2;
	int sub_starty = 3, sub_startx = 1;
	WINDOW *sub_window = derwin(window, sub_height, sub_width, sub_starty, sub_startx);
	wrefresh(sub_window);

	/* Create menu items */ 
	ITEM **menu_items = (ITEM **) calloc(ARRAY_SIZE(menu_labels) + 1, sizeof(ITEM *));
	for (int i = 0; i < ARRAY_SIZE(menu_labels); ++i) {
		menu_items[i] = new_item(menu_labels[i], "");
	}
	menu_items[ARRAY_SIZE(menu_labels)] = (ITEM *)NULL;
	
	/* Create menu */
	MENU *menu = new_menu(menu_items);
	set_menu_win(menu, window);
	set_menu_sub(menu, sub_window);
	set_menu_format(menu, ARRAY_SIZE(menu_labels), 1);
	set_menu_mark(menu, " * ");	

	/* Create border */
	box(window, 0, 0);
	wattron(window, COLOR_PAIR(1));
	char *title = "Snake.c";
	mvwprintw(window, 1, (width - strlen(title)) / 2, "%s", title);
	wattroff(window, COLOR_PAIR(1));
	refresh();

	/* Create line */
	mvwaddch(window, sub_starty - 1, 0, ACS_LTEE);
	mvwhline(window, sub_starty - 1, 1, ACS_HLINE, width - 2);
	mvwaddch(window, sub_starty - 1, width - 1, ACS_RTEE);

	post_menu(menu);
	wrefresh(window);

	/* Create context */ 
	Context *context = malloc(sizeof(*context));
	if (!context) {
		fprintf(stderr, "Failed to allocate memory for context.");
	}
	context->main = window;
	context->sub = sub_window;
	context->menu = menu;
	context->item = menu_items;

	/* Exit prompt */ 
	attron(COLOR_PAIR(1));
	mvprintw(LINES - 2, 0, "F1 to Exit");
	attroff(COLOR_PAIR(1));		
	refresh();

	GameState state = STATE_MENU;
	Snake *snake = NULL;
	Apple *apple = NULL;

	/* Main loop */
	int ch;
	while (state != STATE_EXIT) {
		wtimeout(window, 100);
		ch = wgetch(window);
		switch (state) {
			case STATE_MENU:
				switch (ch) {
					case KEY_UP:
						menu_driver(menu, REQ_UP_ITEM);
						break;

					case KEY_DOWN:
						menu_driver(menu, REQ_DOWN_ITEM);
						break;

					case 10: /* ENTER key */
						switch (label_to_choice(item_name(current_item(menu)))) {
							case ITEM_START:
								state = STATE_PLAYING;

								/* Create snake */
								snake = malloc(sizeof(*snake));
								snake->direction = (Vector2){0, 0};
								snake->body = '#';
								
								Node *head = malloc(sizeof(*head));
								head->position = (Vector2){2, 2};
								head->prev = NULL;
								head->next = NULL;

								snake->head = head;
								snake->tail = head;
										
								/* Create apple */ 
								apple = malloc(sizeof(*apple));
								int y = rand() % (height - 1) + 1;
								int x = rand() % (width - + 1) + 1; 
								apple->position = (Vector2){y, x};
								apple->body = 'o';

								/* Wipe window */
								int height, width;
								getmaxyx(window, height, width);
								for (int y = 3; y < height - 1; y++) {
									for (int x = 1; x < width - 1; x++) {
										mvwaddch(window, y, x, ' ');
									}
								}
								break;

							case ITEM_EXIT:
								state = STATE_EXIT;
								break;
						}
						break;
				}
				break;

			case STATE_PLAYING:
				switch (ch) {
				case KEY_UP:
					snake->direction = (Vector2){-1, 0};
					break;
				case KEY_DOWN:
					snake->direction = (Vector2){1, 0};
					break;
				case KEY_LEFT:
					snake->direction = (Vector2){0, -1};
					break;
				case KEY_RIGHT:
					snake->direction = (Vector2){0, 1};
					break;
				case 'p':
					state = STATE_PAUSE;
					break;
				case 'q':
					state = STATE_MENU;
					
					break;
			}

			/* Wrap around map */
			if (snake->head->position.x > width - 3) {
				snake->head->position.x = 2;
			} else if (snake->head->position.x < 2) {
				snake->head->position.x = width - 3;
			}

			if (snake->head->position.y > height - 3) {
				snake->head->position.y = 3;
			} else if (snake->head->position.y < 4) {
				snake->head->position.y = height - 3;
			}
				
			/* Wipe window */
			int height, width;
			getmaxyx(window, height, width);
			for (int y = 3; y < height - 1; y++) {
				for (int x = 1; x < width - 1; x++) {
					mvwaddch(window, y, x, ' ');
				}
			}

			/* Move snake nodes */
			Node *current = snake->tail;
			while (current->prev != NULL) {
				current->position = current->prev->position;
				current = current->prev;
			}

			/* Move snake */
			snake->head->position = (Vector2){
				snake->head->position.y += snake->direction.y, 
				snake->head->position.x += snake->direction.x
			};
			
			/* Paint snake */
			current = snake->head;
			while (current != NULL) {
				mvwaddch(window, current->position.y, current->position.x, snake->body);
				current = current->next;
			}

			/* Paint apples */
			mvwaddch(window, apple->position.y, apple->position.x, apple->body);

			/* Check for self collion */
			current = snake->head->next;
			//if (current == NULL) break;
			while (current != NULL) {
				if ((snake->head->position.y == current->position.y) &&
					(snake->head->position.x == current->position.x)) {
					state = STATE_MENU;
					break;
				}
				current = current->next;
			}

			/* Eat apple */ 
			if ((snake->head->position.y == apple->position.y) &&
				(snake->head->position.x == apple->position.x)) {
				/* Grow snake */
				Node *prev = snake->tail;
				Node *current = malloc(sizeof(*current));
				current->position = (Vector2){
					prev->position.y + (snake->direction.y * -1),
					prev->position.x + (snake->direction.x * -1)
				};
				current->next = NULL;
				current->prev = prev;
				prev->next = current;
				snake->tail = current;
				int y = rand() % (height - 1) + 1;
				int x = rand() % (width - + 1) + 1; 
				apple->position = (Vector2){y, x};
			}
			
			free(current);
			break;

		case STATE_PAUSE:
			switch (ch) {
			case 'r':
				state = STATE_PLAYING;
				break;
			case 'q':
				state = STATE_MENU;
				
				break;
			}
			break;
		}
		refresh();
		wrefresh(window);
	}

	/* Clean up */
	refresh();
	wrefresh(window);
	wrefresh(sub_window);

	unpost_menu(menu);
	free_menu(menu);
	for (int i = 0; i < ARRAY_SIZE(menu_labels); ++i) {
		free_item(menu_items[i]);
	}

	delwin(window);
	endwin();

	return EXIT_SUCCESS;
}
