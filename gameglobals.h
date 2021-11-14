#include "console.h"
#include "centipede.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"

#define BUF_SIZE 10
#define UPKEEP_ROW 0
#define SCORE_POS 29
#define LIVES_POS 45
#define BULLET_SIZE 1

void *runUpkeep(void *data);