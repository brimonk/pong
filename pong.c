/* 
 * Brian Chrzanowski's Terminal Pong
 * Fri Dec 02, 2016 17:00
 */

#include <ncurses.h>
#include <string.h>
#include <unistd.h>

#define DELAY 30000

struct paddle {
    /* paddle variables */
    int x;
    int y;    /* y is the 'top' of the paddle */
    int len;
    int score;
} paddle_t;

struct ball {
    /* ball variables */
	int ball_x;
    int ball_y;
	int next_x;
    int next_y;
    int ball_x_vel;
    int ball_y_vel;
} ball_t;

struct dimensions {
    int max_x;
    int max_y;
} dimensions_t;

void draw_ball(struct ball *input);
void draw_paddle(struct paddle *paddle);
void draw_score(struct paddle *inpt_paddle, struct dimensions *wall);
void paddle_collisions(struct ball *inpt_ball, struct paddle *inpt_paddle);
void paddle_pos(struct paddle *pddl, struct dimensions *wall, char dir);

int wall_collisions(struct ball *usr_ball, struct dimensions *walls);
int kbdhit();

int main(int argc, char **argv)
{
	/* initialize curses */
	initscr();
	noecho();
	curs_set(0);

	struct dimensions walls = { 0 };
	getmaxyx(stdscr, walls.max_y, walls.max_x); /* get dimensions */

	struct paddle usr_paddle = { 0 };

	/* set the paddle variables */
	usr_paddle.x = 5;
	usr_paddle.y = 11;
	usr_paddle.len = walls.max_y / 4;
	usr_paddle.score = 0;

	/* now that we can get term dimensions, we can make a ball */
	struct ball usr_ball = { 0 };

	/* set the initial values for the ball */
	usr_ball.ball_x = walls.max_x / 2;
	usr_ball.ball_y = walls.max_y / 2;
	usr_ball.next_x = 0;
	usr_ball.next_y = 0;
	usr_ball.ball_x_vel = 1;
	usr_ball.ball_y_vel = 1;

	/* we actually have to store the user's keypress somewhere... */
	int keypress = 0;
	int run = 1;
	nodelay(stdscr, TRUE);
	scrollok(stdscr, TRUE);

	while (run) {
		while (kbdhit()) {
			getmaxyx(stdscr, walls.max_y, walls.max_x);
			clear(); /* clear screen of all printed chars */

			draw_ball(&usr_ball);
			draw_paddle(&usr_paddle);
			draw_score(&usr_paddle, &walls);
			refresh(); /* draw to term */
			usleep(DELAY);

			/* set next positions */
			usr_ball.next_x = usr_ball.ball_x + usr_ball.ball_x_vel;
			usr_ball.next_y = usr_ball.ball_y + usr_ball.ball_y_vel;

			/* check for collisions */
			paddle_collisions(&usr_ball, &usr_paddle);
			if (wall_collisions(&usr_ball, &walls)) {
				run = 0;
				break;
			}
		}

		/* we fell out, get the key press */
		keypress = getch();


		switch (keypress) {
		case 'j':
		case 'k':
			paddle_pos(&usr_paddle, &walls, keypress);
			break;

		case 'p': /* pause functionality, because why not */
			mvprintw(1, 0, "PAUSE - press any key to resume");
			while (!kbdhit()) {
				usleep(DELAY);
			}
			break;
		case 'q':
			run = 0;
			break;
		}
	}

	endwin();

	printf("GAME OVER\nFinal Score: %d\n", usr_paddle.score);

	return 0;
}

/*
 * function : paddle_pos
 * purpose  : have a function that will return a proper 'y' value for the paddle
 * input    : struct paddle *inpt_paddle, struct dimensions *wall, char dir
 * output   : void
 */

void paddle_pos(struct paddle *pddl, struct dimensions *wall, char dir)
{
	if (dir == 'j') { /* moving down */
		if (pddl->y + pddl->len + 1 <= wall->max_y)
			pddl->y++;
	} else {          /* moving up (must be 'k') */
		if (pddl->y - 1 >= 0)
			pddl->y--;

	}

	return;
}

/*
 * function : wall_collisions
 * purpose  : to check for collisions on the terminal walls
 * input    : struct ball *, struct dimensions *
 * output   : nothing (stored within the structs)
 */
int wall_collisions(struct ball *usr_ball, struct dimensions *walls)
{
	/* check if we're supposed to leave quick */
	if (usr_ball->next_x < 0) {
		return 1;
	}

	/* check for X */
	if (usr_ball->next_x >= walls->max_x) {
		usr_ball->ball_x_vel *= -1;
	} else {
		usr_ball->ball_x += usr_ball->ball_x_vel;
	}

	/* check for Y */
	if (usr_ball->next_y >= walls->max_y || usr_ball->next_y < 0) {
		usr_ball->ball_y_vel *= -1;
	} else {
		usr_ball->ball_y += usr_ball->ball_y_vel;
	}

	return 0;
}

/* -------------------------------------------------------------------------- */

void paddle_collisions(struct ball *inpt_ball, struct paddle *inpt_paddle)
{
	/* 
	* simply check if next_% (because we set the next_x && next_y first) 
	* is within the bounds of the paddle's CURRENT position
	*/

	if (inpt_ball->next_x == inpt_paddle->x) {
		if (inpt_paddle->y <= inpt_ball->ball_y &&
			inpt_ball->ball_y <= 
			inpt_paddle->y + inpt_paddle->len) {

			inpt_paddle->score++;
			inpt_ball->ball_x_vel *= -1;
		}
	}

	return;
}

/* -------------------------------------------------------------------------- */

/*
 * functions : draw_ball && draw_paddle
 * purpose   : condense the drawing functions to functions
 * input     : struct ball * && struct paddle *
 * output    : void
 */
void draw_ball(struct ball *input)
{
	mvprintw(input->ball_y, input->ball_x, "O");
	return;
}

void draw_paddle(struct paddle *paddle)
{
	int i;

	for (i = 0; i < paddle->len; i++)
		mvprintw(paddle->y + i, paddle->x, "|");

	return;
}

void draw_score(struct paddle *inpt_paddle, struct dimensions *wall)
{
	mvprintw(0, wall->max_x / 2 - 7, "Score: %d", inpt_paddle->score);
}

/* -------------------------------------------------------------------------- */

/*
 * function : kbdhit
 * purpose  : find out if we've got something in the input buffer
 * input    : void
 * output   : 0 on none, 1 on we have a key
 */

int kbdhit()
{
	int key = getch();

	if (key != ERR) {
		ungetch(key);
		return 0;
	} else {
		return 1;
	}
}
