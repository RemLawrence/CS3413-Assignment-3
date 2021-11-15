/**********************************************************************
  Module: constants.h
  Author: Micah Hanmin Wang

  This is a constant library, storing all the constants across all the 
  files needed.
  Unfortunately, I don't time to integrate this library for the rest of 
  my files, but at least I have the intent to use a constant library 
  like this.

**********************************************************************/


/****************CENTIPEDE.C****************/
#define KEY_W_PREESSED 'w'
#define KEY_A_PREESSED 'a'
#define KEY_S_PREESSED 's'
#define KEY_D_PREESSED 'd'
#define SPACE_PREESSED ' '
#define KEY_Q_PREESSED 'q'
#define PLAYER_BOUNDARY_ROW 17
#define ENEMY_HEIGHT 2
#define ENEMY_WIDTH 80
#define PLAYER_START_ROW 20
#define PLAYER_START_COL 36
#define PLAYER_INIT_LIVES 4
char *GAME_BOARD[] = {
"                   Score:    0     Lives:    4",
"=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-centipiede!=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"",
"",
"",
"",
"",
"",
"", 
"" };



/****************GAMEGLOBALS.H****************/
#define BUF_SIZE 10
#define UPKEEP_ROW 0
#define SCORE_POS 29
#define LIVES_POS 45
#define BULLET_SIZE 1




/****************PLAYER.H****************/
#define PLAYER_ANIM_TILES 3
#define PLAYER_ANIM_TICKS 30
#define PLAYER_WIDTH 5 
#define PLAYER_HEIGHT 4 



/****************ENEMY.H****************/
#define ENEMY_HEIGHT 2
#define ENEMY_WIDTH 80
#define ENEMY_BODY_ANIM_TILES 4 
#define NUM_THREADS 1
#define ENEMY_MIN_WIDTH 4
#define LOWER_ROW_BOUNDARY 14
#define COL_BOUNDARY 80
#define LAST_ROW 14
#define ENEMY_FIRST_ROW 2
#define ENEMY_SPEED 40
char* ENEMY_BODY_LEFT[ENEMY_BODY_ANIM_TILES][ENEMY_HEIGHT] = 
{
    {"@|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||",
    "=;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,"},
    {"@||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^",
    "=;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;"},
    {"@|^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|",
    "=,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;"},
    {"-^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^||",
    "=;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;"}
};

char* ENEMY_BODY_RIGHT[ENEMY_BODY_ANIM_TILES][ENEMY_HEIGHT] = 
{
    {"|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||@",
    ";;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,="},
    {"^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^||-",
    ";;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;="},
    {"|^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|@",
    ",;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;="},
    {"||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^@",
    ";,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;="}
};



/****************BULLET.H****************/
#define BULLET_SIZE 1
#define BULLET_ANIM_TILES 1
#define UPPER_BOUNDARY 2
char* PLAYER_BULLET[BULLET_SIZE][BULLET_ANIM_TILES] = 
{
   {"|"}
};
char* ENEMY_BULLET[BULLET_SIZE][BULLET_ANIM_TILES] = 
{
   {"."}
};


/****************LLIST.H****************/
#define ENEMY_MIN_WIDTH 4
#define MIN_SPAWN_TICK 8000
#define MAX_SPAWN_TICK 10000